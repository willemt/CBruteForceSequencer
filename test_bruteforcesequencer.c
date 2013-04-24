/*
 * =====================================================================================
 *
 *       Filename:  test_bruteforcesequencer.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/19/10 00:35:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"
#include <stdbool.h>
#include "arrayqueue.h"
#include "linked_list_hashmap.h"
#include "bruteforcesequencer.h"

static int __dummy_sobj_release(void* obj __attribute__((__unused__)))
{
    return 1;
}

void TestBruteforcesequencer_InitIsEmpty(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    CuAssertTrue(tc, NULL != bfs);
    CuAssertTrue(tc, 0 == bfsequencer_get_num_sobj(bfs));
    CuAssertTrue(tc, 0 == bfsequencer_get_last_offered_seqn(bfs));
}

void TestBruteforcesequencer_OfferSobj(
    CuTest * tc
)
{
    bfs_t *bfs;
    int object = 10;

    bfs = bfsequencer_new();

    CuAssertTrue(tc, NULL != bfs);
    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object, 15));
    CuAssertTrue(tc, 0 == bfsequencer_get_last_offered_seqn(bfs));
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj(bfs));
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj_all(bfs));
}

void TestBruteforcesequencer_OfferSobjAgainAfterSeqnIncreasesIsValid(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object = 10;

    CuAssertTrue(tc, NULL != bfs);
    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object, 15));
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj_all(bfs));

    bfsequencer_increase_seqn(bfs);

    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object, 15));
    CuAssertTrue(tc, 2 == bfsequencer_get_num_sobj_all(bfs));
}

void TestBruteforcesequencer_GetLastAckedWithNoAckedIsInvalid(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    void *obj;

    CuAssertTrue(tc, NULL != bfs);
    obj = bfsequencer_get_last_acked_sobj_from_id(bfs, 15); //, &seqn);
    CuAssertTrue(tc, !obj);
//    CuAssertTrue(tc, 0 == seqn);
}

#if 0
void TxestBruteforcesequencer_OfferIncrementsSeqn(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    int object2 = 10;

    int seqn;

    CuAssertTrue(tc, bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);

    CuAssertTrue(tc, 1 == bfsequencer_get_last_offered_seqn(bfs));

    bfsequencer_offer_sobj(bfs, &object2, 16);

    CuAssertTrue(tc, 1 == bfsequencer_get_last_offered_seqn(bfs));

    bfsequencer_offer_sobj(bfs, &object2, 15);

    CuAssertTrue(tc, 2 == bfsequencer_get_last_offered_seqn(bfs));
}
#endif

void TestBruteforcesequencer_OfferringSobjMoreThanOnceInOneSeqnFails(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    CuAssertTrue(tc, NULL != bfs);
    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object1, 15));
    CuAssertTrue(tc, 0 == bfsequencer_get_last_offered_seqn(bfs));

    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object1, 16));
    CuAssertTrue(tc, 0 == bfsequencer_get_last_offered_seqn(bfs));

    CuAssertTrue(tc, 0 == bfsequencer_offer_sobj(bfs, &object1, 15));
    CuAssertTrue(tc, 0 == bfsequencer_get_last_offered_seqn(bfs));
}

void TestBruteforcesequencer_IncreaseSeqn(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    CuAssertTrue(tc, NULL != bfs);
    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object1, 15));
    CuAssertTrue(tc, 0 == bfsequencer_get_last_offered_seqn(bfs));

    bfsequencer_increase_seqn(bfs);

    CuAssertTrue(tc, 1 == bfsequencer_get_last_offered_seqn(bfs));
}

void TestBruteforcesequencer_GetLastAcked(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    void *obj;

    CuAssertTrue(tc, NULL != bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj(bfs));
    CuAssertTrue(tc, !bfsequencer_get_last_acked_sobj_from_id(bfs, 15));

    bfsequencer_ack_seqn(bfs, 0);
    obj = bfsequencer_get_last_acked_sobj_from_id(bfs, 15); //, &seqn);
    CuAssertTrue(tc, &object1 == obj);
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj(bfs));
}

#if 0
void TxestBruteforcesequencer_GetLastObjectFromID(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    int object2 = 10;

    void *obj;

    CuAssertTrue(tc, NULL != bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_offer_sobj(bfs, &object2, 15);

    obj = bfsequencer_get_last_sobj_from_id(bfs, 15);
    CuAssertTrue(tc, &object2 == obj);
}
#endif

void TestBruteforcesequencer_GetLastObjectFromID(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    int object2 = 10;

    void *obj;

    CuAssertTrue(tc, NULL != bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &object2, 15);

    int seqn = bfsequencer_get_last_offered_seqn(bfs);

    obj = bfsequencer_get_last_sobj_from_id(bfs, 15, seqn);
    CuAssertTrue(tc, &object1 == obj);
}

void TestBruteforcesequencer_GetLastObjectFromIDInvalidIfOnlyOne(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object1 = 10;

    void *obj;

    CuAssertTrue(tc, NULL != bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);

    int seqn = bfsequencer_get_last_offered_seqn(bfs);

    obj = bfsequencer_get_last_sobj_from_id(bfs, 15, seqn);
    CuAssertTrue(tc, !obj);
}

void TestBruteforcesequencer_AckReducesArchives(
    CuTest * tc
)
{
    bfs_t *bfs;
    int object1 = 10;

    bfs = bfsequencer_new();
    bfsequencer_set_free_func(bfs, __dummy_sobj_release);

    CuAssertTrue(tc, NULL != bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &object1, 15);
    bfsequencer_increase_seqn(bfs);
    CuAssertTrue(tc, 6 == bfsequencer_get_last_offered_seqn(bfs));
    CuAssertTrue(tc, 6 == bfsequencer_get_num_sobj_all(bfs));

    bfsequencer_ack_seqn(bfs, 2);
    CuAssertTrue(tc, 6 == bfsequencer_get_last_offered_seqn(bfs));
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj(bfs));
    CuAssertTrue(tc, 4 == bfsequencer_get_num_sobj_all(bfs));

    bfsequencer_ack_seqn(bfs, 5);
    CuAssertTrue(tc, 6 == bfsequencer_get_last_offered_seqn(bfs));
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj(bfs));
    CuAssertTrue(tc, 1 == bfsequencer_get_num_sobj_all(bfs));
}

void TestBruteforcesequencer_RemoveSobjRemovesSobj(
    CuTest * tc
)
{
    bfs_t *bfs;

    bfs = bfsequencer_new();

    int object = 10;

    CuAssertTrue(tc, 1 == bfsequencer_offer_sobj(bfs, &object, 15));

    bfsequencer_remove_sobj(bfs,15);

    int seqn = bfsequencer_get_last_offered_seqn(bfs);

    void* obj = bfsequencer_get_last_sobj_from_id(bfs, 15, seqn);

    CuAssertTrue(tc, NULL == obj);
    CuAssertTrue(tc, 0 == bfsequencer_get_num_sobj(bfs));
    CuAssertTrue(tc, 0 == bfsequencer_get_num_sobj_all(bfs));
}

void TestBruteforcesequencer_IteratorIteratesAll(
    CuTest * tc
)
{
    bfs_t *bfs;
    bfs_datasend_iterator_t iter;
    int o1 = 11;
    int o2 = 12;
    int o3 = 13;

    bfs = bfsequencer_new();

    bfsequencer_offer_sobj(bfs, &o1, 11);
    bfsequencer_offer_sobj(bfs, &o2, 12);
    bfsequencer_offer_sobj(bfs, &o3, 13);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_iterator_datasends_to_send(bfs,&iter);

    CuAssertTrue(tc, 1 == bfsequencer_iterator_datasends_to_send_has_next(bfs,&iter));
    CuAssertTrue(tc, &o1 == bfsequencer_iterator_datasends_to_send_next(bfs,&iter));
    CuAssertTrue(tc, &o2 == bfsequencer_iterator_datasends_to_send_next(bfs,&iter));
    CuAssertTrue(tc, &o3 == bfsequencer_iterator_datasends_to_send_next(bfs,&iter));
    CuAssertTrue(tc, 0 == bfsequencer_iterator_datasends_to_send_has_next(bfs,&iter));
}

void TestBruteforcesequencer_IteratorIteratesAllButAcked(
    CuTest * tc
)
{
    bfs_t *bfs;
    bfs_datasend_iterator_t iter;
    int o1 = 11;
    int o2 = 12;
    int o3 = 13;

    bfs = bfsequencer_new();

    bfsequencer_offer_sobj(bfs, &o1, 11);
    bfsequencer_offer_sobj(bfs, &o2, 12);
    bfsequencer_offer_sobj(bfs, &o3, 13);
    bfsequencer_increase_seqn(bfs);
    bfsequencer_offer_sobj(bfs, &o2, 12);
    bfsequencer_offer_sobj(bfs, &o3, 13);
    bfsequencer_ack_seqn(bfs, 0);
    bfsequencer_iterator_datasends_to_send(bfs,&iter);

    CuAssertTrue(tc, 1 == bfsequencer_iterator_datasends_to_send_has_next(bfs,&iter));
    CuAssertTrue(tc, &o2 == bfsequencer_iterator_datasends_to_send_next(bfs,&iter));
    CuAssertTrue(tc, &o3 == bfsequencer_iterator_datasends_to_send_next(bfs,&iter));
    CuAssertTrue(tc, 0 == bfsequencer_iterator_datasends_to_send_has_next(bfs,&iter));
}
