
typedef struct
{
    void *in;
} bfs_t;

typedef struct
{
    hashmap_iterator_t hmap_iter;
} bfs_datasend_iterator_t;

bfs_t *bfsequencer_new();

void bfsequencer_set_free_func( bfs_t * bfs, int (*func) (void *));

int bfsequencer_get_num_sobj( bfs_t * self);

int bfsequencer_get_num_sobj_all( bfs_t * self);

void bfsequencer_remove_sobj( bfs_t * self, unsigned long id);

int bfsequencer_get_last_offered_seqn( bfs_t * self);

int bfsequencer_offer_sobj( bfs_t * self, void *sobj, unsigned long id);

void *bfsequencer_get_last_acked_sobj_from_id( bfs_t * self, unsigned long id);

void *bfsequencer_get_last_sobj_from_id( bfs_t * self, unsigned long id, const int seqn);

void bfsequencer_ack_seqn( bfs_t * self, const int ackseqn);

void bfsequencer_increase_seqn( bfs_t * self);

int bfsequencer_iterator_datasends_to_send_has_next( bfs_t * self __attribute__((__unused__)), bfs_datasend_iterator_t* iter __attribute__((__unused__)));

void *bfsequencer_iterator_datasends_to_send_next( bfs_t * self, bfs_datasend_iterator_t* iter);

void bfsequencer_iterator_datasends_to_send( bfs_t * self, bfs_datasend_iterator_t* iter);
