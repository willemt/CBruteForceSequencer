/*
 
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/*
The brute force sequencer's job:

Ensure that the client has the exact same state we have
 
Guarantees:
1. Send newestState(object,seqn) until it is ACK(seqn) by client
2. Remember the last acked state; so that:
     a. The data we send is message(delta(newest,lastacked),seqn)

Functionality:
a) be notified of objects that have been updated and need to be sent at this seqn
b) retrieve an iterator over the states that need to be sent (to cover off guarantee 1)
c) increment current seqn
d) retrieve last acked state of an object
e) get the last state of an object
f) update the system after an ACK(seqn) is received

Example
The system sends state S with appended sequence number N. The client has acked S0.
The last 6 sends happen like this:
S1, S2, S3, S4, null, null, S6

The delta in this case is like this:
delta(0,1), delta(0,2), delta(0,3), delta(0,3), delta(0,3), delta(0,6)

*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "arrayqueue.h"
#include "linked_list_hashmap.h"
#include "bruteforcesequencer.h"

typedef struct
{
    /* we hold the history of the states here, so we can delta compress */
    arrayqueue_t *states;

} sobstates_t;

typedef struct
{
    int isAcked;
    int seqn;
    void *sobj;
} sob_t;

typedef struct
{
    arrayqueue_t *states;

    hashmap_t *sobjs;

    /* the latest seqn we are sending */
    int seqn_newest;

    /* for freeing the sobj */
    int ( *sobj_free_func) ( void *);
} private_t;

#define in(x) ((private_t*)x->in)


/* Integer object
 * Please don't be negative */
static unsigned long __ulong_hash(
    const void *e1
)
{
    const long i1 = (unsigned long) e1;

    assert(i1 >= 0);
    return i1;
}

static long __ulong_compare(
    const void *e1,
    const void *e2
)
{
    const long i1 = (unsigned long) e1, i2 = (unsigned long) e2;

    return i1 - i2;
}

/**
 * @return new sequencer */
bfs_t *bfsequencer_new(
)
{
    bfs_t *me;

    me = calloc(1,sizeof(bfs_t));
    me->in = calloc(1,sizeof(private_t));
    /* a register of sequenced objects */
    in(me)->sobjs = hashmap_new(__ulong_hash, __ulong_compare, 11);

//    hashmap_t *dic;
//    dic = hashmap_initalloc(&teaObjUlong);
//    arrayqueue_offer(in(me)->states, dic);

    return me;
}

static void __sobj_release(
    bfs_t * bfs,
    sob_t * sobj
)
{
    /* free the user data if we can */
    if (in(bfs)->sobj_free_func)
    {
        in(bfs)->sobj_free_func(sobj->sobj);
    }
    free(sobj);
}

void bfsequencer_set_free_func(
    bfs_t * bfs,
    int (*func) (void *)
)
{
    in(bfs)->sobj_free_func = func;
}

/**
 * @return number of sequenced objects */
int bfsequencer_get_num_sobj(
    bfs_t * me
)
{
    return hashmap_count(in(me)->sobjs);
}

static void __count_queue_count(
    arrayqueue_t * queue,
    int *accumatlor
)
{
    *accumatlor += arrayqueue_count(queue);
}

/**
 * @return number of sequenced objects, even if the objects are duplicates */
int bfsequencer_get_num_sobj_all(bfs_t * me)
{
    hashmap_iterator_t iter;
    int size = 0;

    for (hashmap_iterator(in(me)->sobjs, &iter);
            hashmap_iterator_has_next(in(me)->sobjs, &iter);)
    {
        __count_queue_count(
                hashmap_iterator_next_value(in(me)->sobjs, &iter),&size);
    }

    return size;
}

/**
 * Remove this sequenced object
 * @param id The ID of the sequenced object to be removed */
void bfsequencer_remove_sobj(
    bfs_t * me,
    unsigned long id
)
{
    /* the states that this sequenced object has */
    arrayqueue_t *ss;

    /* remove from register */
    if ((ss = hashmap_remove(in(me)->sobjs, (void *) id)))
    {
        /* empty all the sequences */
        while (!arrayqueue_is_empty(ss))
        {
            __sobj_release(me, arrayqueue_poll(ss));
        }
        arrayqueue_free(ss);
    }
}

/**
 * @return last offered sequence number for this bfs */
int bfsequencer_get_last_offered_seqn(
    bfs_t * me
)
{
    return in(me)->seqn_newest;
}

/**
 * Register new sequenced object
 * @param id ID of the object to be registered */
static arrayqueue_t* __register_new_sequenced_object(
    bfs_t * me,
    unsigned long id)

{
    arrayqueue_t *ss;

    ss = arrayqueue_new();
    hashmap_put(in(me)->sobjs, (void *) id, ss);

    return ss;
}

/**
 * Add sequenced object
 *
 * Fail when an obj is offered more than once in one seqnumber
 * 
 * @param id The ID of the object
 * @return 0 on error; otherwise 1. */
int bfsequencer_offer_sobj(
    bfs_t * me,
    void *sobj,
    unsigned long id
)
{
    /* the states that this sequenced object has */
    arrayqueue_t *ss;
    sob_t *sob;

    /* check if we have this sequenced object */
    if (!(ss = hashmap_get(in(me)->sobjs, (void *) id)))
    {
        ss = __register_new_sequenced_object(me,id);
    }
    else
    {
        sob = arrayqueue_peektail(ss);

        /*  make sure we aren't adding twice in one sequence */
        if (sob && sob->seqn == bfsequencer_get_last_offered_seqn(me))
        {
            return 0;
        }
    }

    /* append state */
    sob = calloc(1,sizeof(sob_t));
    sob->sobj = sobj;
    sob->seqn = in(me)->seqn_newest;
    arrayqueue_offer(ss, sob);
    return 1;
}

/**
 * Get the last state of this sobj that was acked
 * @param id The ID of the object
 * @return null if none have been acked */
void *bfsequencer_get_last_acked_sobj_from_id(
    bfs_t * me,
    unsigned long id
)
{
    arrayqueue_t *ss;
    sob_t *sobj;

    if (!(ss = hashmap_get(in(me)->sobjs, (void *) id)))
    {
        return NULL;
    }

    /* front of the queue has most recent acked */
    sobj = arrayqueue_peek(ss);
    if (sobj && sobj->isAcked)
    {
        return sobj->sobj;
    }
    else
    {
        return NULL;
    }
}

/**
 * @param id The ID of the object
 * @return last sobj before this seqn */
void *bfsequencer_get_last_sobj_from_id(
    bfs_t * me,
    unsigned long id,
    const int seqn
)
{
    arrayqueue_iterator_t iter;
    arrayqueue_t *ss;

    if (!(ss = hashmap_get(in(me)->sobjs, (void *) id)))
    {
        return NULL;
    }

    /* look backwards... */
    for (arrayqueue_iterator_reverse(ss,&iter);
         arrayqueue_iterator_has_next_reverse(ss,&iter);)
    {
        sob_t *sobj;
        
        sobj = arrayqueue_iterator_next_reverse(ss,&iter);

        if (sobj->seqn < seqn)
        {
            return sobj->sobj;
        }
    }

    return NULL;
}

/**
 * Acknowledged receipt, so delete archives up to sequence number
 * @param ackseqn The sequence number
 */
void bfsequencer_ack_seqn(
    bfs_t * me,
    const int ackseqn
)
{
    hashmap_iterator_t iter;

    assert(in(me)->seqn_newest >= ackseqn);

    /*  iterate throught all sobjs */
    for (hashmap_iterator(in(me)->sobjs,&iter);
            hashmap_iterator_has_next(in(me)->sobjs,&iter);)
    {
        arrayqueue_t *ss;
        sob_t *sobj;

        ss = hashmap_iterator_next_value(in(me)->sobjs,&iter);

        do {
            sobj = arrayqueue_peek(ss);

            if (sobj->seqn == ackseqn)
            {
                sobj->isAcked = 1;
                break;
            }
            else
            {
                __sobj_release(me, arrayqueue_poll(ss));
            }
        } while (1);

#if 0
        for (sobj = arrayqueue_peek(ss);
             sobj->seqn <= ackseqn; sobj = arrayqueue_peek(ss))
        {
            if (sobj->seqn == ackseqn ||
                /*  if we haven't sent that much of this sobj.. */
                /*  assume that it has been acked */
                1 == arrayqueue_count(ss))
            {
                sobj->isAcked = 1;
                sobj->seqn = ackseqn;
                break;
            }
            else
            {
                __sobj_release(me, arrayqueue_poll(ss));
            }
        }
#endif
    }

#if 0
    for (in(me)->seqn_oldest;
         in(me)->seqn_oldest < ackseqn; in(me)->seqn_oldest++)
    {
        hashmap_t *dic;

        dic = arrayqueue_poll(in(me)->states);
        tea_iter_forall(arrayqueue_iter(in(me)->states),
                        in(me)->sobj_free_func);
        hashmap_freeall(dic);
    }
#endif
}

/**
 * Increase the sequence number */
void bfsequencer_increase_seqn(
    bfs_t * me
)
{
#if 0
    hashmap_t *dic;

    dic = hashmap_initalloc(&teaObjUlong);
    arrayqueue_offer(in(me)->states, dic);
    in(me)->seqn_newest++;
#endif
    in(me)->seqn_newest++;
}

int bfsequencer_iterator_datasends_to_send_has_next(
    bfs_t * me,
    bfs_datasend_iterator_t* iter 
)
{
    while (hashmap_iterator_has_next(in(me)->sobjs, &iter->hmap_iter))
    {
        sob_t *sobj;
        arrayqueue_t *ss;

        ss = hashmap_iterator_peek_value(in(me)->sobjs,&iter->hmap_iter); 

        assert(ss);
        assert(0 < arrayqueue_count(ss));

        sobj = arrayqueue_peektail(ss);

        /*  if it's been acked we don't need to send it! */
        if (sobj->isAcked)
        {
            hashmap_iterator_next(in(me)->sobjs,&iter->hmap_iter);
            continue;
        }

        return 1;
    }

    return 0;
//    return tea_iter_hasNext(sobjs) ? 1 : 0;
}

void *bfsequencer_iterator_datasends_to_send_next(
    bfs_t * me,
    bfs_datasend_iterator_t* iter
)
{
    if (bfsequencer_iterator_datasends_to_send_has_next(me,iter))
    {
        sob_t *sobj;
        arrayqueue_t *ss;
        
        ss = hashmap_iterator_next_value(in(me)->sobjs,&iter->hmap_iter);
        sobj = arrayqueue_peektail(ss);
        return sobj->sobj;
    }

    return NULL;
}

void bfsequencer_iterator_datasends_to_send(
    bfs_t * me,
    bfs_datasend_iterator_t* iter
)
{
    hashmap_iterator(in(me)->sobjs,&iter->hmap_iter);
}
