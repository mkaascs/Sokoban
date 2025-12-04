#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "../game.h"

#define MAX_STATES 5000000

static const int DX[4] = {-1, 1, 0, 0};
static const int DY[4] = {0, 0, 1, -1};
static const char DIR_CH[4] = {'L','R','U','D'};

typedef struct {
    Level lvl;
    int g;
    int h;
    int f;
    int parent;
    char move;
} Node;

typedef struct {
    uint64_t* keys;
    uint8_t* used;
    size_t cap;
} HashSet64;

static HashSet64* hs_create(size_t cap_pow2) {
    HashSet64* hs = malloc(sizeof(HashSet64));
    hs->cap = cap_pow2;
    hs->keys = calloc(hs->cap,sizeof(uint64_t));
    hs->used = calloc(hs->cap,sizeof(uint8_t));
    return hs;
}

static void hs_free(HashSet64* hs) {
    free(hs->keys); free(hs->used); free(hs);
}

static bool hs_add(HashSet64* hs, uint64_t key) {
    size_t mask=hs->cap-1;
    size_t idx=key&mask;
    for(size_t probe=0; probe<hs->cap; probe++){
        size_t i=(idx+probe)&mask;
        if(!hs->used[i]){ hs->used[i]=1; hs->keys[i]=key; return true;}
        else if(hs->keys[i]==key) return false;
    }
    return false;
}

static void copy_level_shallow(Level* dst, const Level* src) {
    dst->number=src->number;
    bb_init(&dst->walls, src->walls.rows, src->walls.columns);

    memcpy(dst->walls.data, src->walls.data, src->walls.words*sizeof(uint64_t));
    bb_init(&dst->boxes, src->boxes.rows, src->boxes.columns);

    memcpy(dst->boxes.data, src->boxes.data, src->boxes.words*sizeof(uint64_t));
    bb_init(&dst->goals, src->goals.rows, src->goals.columns);

    memcpy(dst->goals.data, src->goals.data, src->goals.words*sizeof(uint64_t));
    bb_init(&dst->player, src->player.rows, src->player.columns);

    memcpy(dst->player.data, src->player.data, src->player.words*sizeof(uint64_t));
}

static void free_level_shallow(Level* lvl) {
    bb_free(&lvl->walls); bb_free(&lvl->boxes);
    bb_free(&lvl->goals); bb_free(&lvl->player);
}

static bool check_win_lvl(Level* lvl) {
    for(int i=0;i<lvl->boxes.words;i++)
        if(lvl->boxes.data[i] & ~lvl->goals.data[i]) return false;

    return true;
}

static bool simple_deadlock(Level* lvl) {
    int rows = lvl->walls.rows, cols = lvl->walls.columns;
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < cols; c++) {
            if (!bb_get(&lvl->boxes,r,c))
                continue;

            if (bb_get(&lvl->goals,r,c))
                continue;

            bool wall_up=(r == 0) ? true : bb_get(&lvl->walls,r-1,c);
            bool wall_down=(r == rows - 1)? true : bb_get(&lvl->walls,r+1,c);
            bool wall_left= (c == 0) ? true : bb_get(&lvl->walls,r,c-1);
            bool wall_right=(c == cols - 1) ? true : bb_get(&lvl->walls,r,c+1);

            if ((wall_up||wall_down) && (wall_left||wall_right))
                return true;
        }
    }

    return false;
}

// --- Эвристика: сумма минимальных манхэттенов ---
static int heuristic(Level* lvl){
    int rows=lvl->walls.rows, cols=lvl->walls.columns;
    int box_count=0, goal_count=0, maxcells=rows*cols;
    int *box_y=malloc(sizeof(int)*maxcells);
    int *box_x=malloc(sizeof(int)*maxcells);
    int *goal_y=malloc(sizeof(int)*maxcells);
    int *goal_x=malloc(sizeof(int)*maxcells);

    for(int r=0;r<rows;r++)
        for(int c=0;c<cols;c++){
            if (bb_get(&lvl->boxes,r,c)) {
                box_y[box_count]=r;
                box_x[box_count]=c;
                box_count++;
            }

            if (bb_get(&lvl->goals,r,c)) {
                goal_y[goal_count]=r;
                goal_x[goal_count]=c;
                goal_count++;
            }
        }

    int h=0;
    for(int i=0;i<box_count;i++){
        int best=INT_MAX;
        for(int j=0;j<goal_count;j++){
            int d=abs(box_y[i]-goal_y[j])+abs(box_x[i]-goal_x[j]);
            if(d<best) best=d;
        }

        h+=best;
    }

    free(box_y);
    free(box_x);
    free(goal_y);
    free(goal_x);
    return h;
}

typedef struct{
    int *data; int size; int capacity; Node* nodes;
} MinHeap;

static MinHeap* heap_create(int cap, Node* nodes){
    MinHeap* h=malloc(sizeof(MinHeap));
    h->data=malloc(sizeof(int)*cap); h->size=0; h->capacity=cap; h->nodes=nodes;
    return h;
}

static void heap_free(MinHeap* h){ free(h->data); free(h); }
static void heap_push(MinHeap* h,int idx){
    h->data[h->size]=idx;
    int i=h->size++;

    while(i>0){ int p=(i-1)/2;
        if(h->nodes[h->data[i]].f<h->nodes[h->data[p]].f) {
            int t=h->data[i];
            h->data[i]=h->data[p];
            h->data[p]=t;
            i=p;
        }

        else break;
    }
}

static int heap_pop(MinHeap* h){
    if(h->size==0) return -1;
    int res=h->data[0]; h->data[0]=h->data[--h->size];
    int i=0;

    while(1){ int l=2*i+1, r=2*i+2, smallest=i;
        if(l<h->size && h->nodes[h->data[l]].f<h->nodes[h->data[smallest]].f)
            smallest=l;

        if(r<h->size && h->nodes[h->data[r]].f<h->nodes[h->data[smallest]].f)
            smallest=r;

        if(smallest!=i) {
            int t=h->data[i];
            h->data[i]=h->data[smallest];
            h->data[smallest]=t;
            i=smallest;
        }

        else break;
    }

    return res;
}

static uint64_t hash_state(Level* lvl){
    uint64_t h=14695981039346656037ULL;
    for(int i=0;i<lvl->boxes.words;i++){
        uint64_t v=lvl->boxes.data[i];
        for(int b=0;b<8;b++){
            uint8_t byte=(v>>(b*8))&0xFF; h^=byte; h*=1099511628211ULL;
        }
    }

    int py=-1,px=-1;
    bb_find_first_bit(&lvl->player,&py,&px);
    if(py<0)
        py=0;

    if(px<0)
        px=0;

    h^=(uint64_t)py; h*=1099511628211ULL;
    h^=(uint64_t)px; h*=1099511628211ULL;
    return h;
}

char* solve_astar(Level* lvl){
    Node* nodes=malloc(sizeof(Node)*MAX_STATES); int nodes_count=0;
    HashSet64* visited=hs_create(1<<22);

    copy_level_shallow(&nodes[0].lvl,lvl);
    nodes[0].g=0;
    nodes[0].h=heuristic(&nodes[0].lvl);
    nodes[0].f=nodes[0].g+nodes[0].h;
    nodes[0].parent=-1;
    nodes[0].move=0;

    nodes_count=1; hs_add(visited,hash_state(&nodes[0].lvl));

    MinHeap* open=heap_create(MAX_STATES,nodes);
    heap_push(open,0);

    int found_idx=-1;
    while(open->size>0){
        int cur_idx=heap_pop(open);
        Node cur=nodes[cur_idx];
        if(check_win_lvl(&cur.lvl)){found_idx=cur_idx; break;}
        int py=-1,px=-1; bb_find_first_bit(&cur.lvl.player,&py,&px);

        for(int d=0;d<4;d++){
            int nx=px+DX[d], ny=py+DY[d];
            if(nx<0||ny<0||nx>=cur.lvl.walls.columns||ny>=cur.lvl.walls.rows)
                continue;

            if(bb_get(&cur.lvl.walls,ny,nx))
                continue;

            bool next_is_box=bb_get(&cur.lvl.boxes,ny,nx);
            if(next_is_box){
                int bx=nx+DX[d],by=ny+DY[d];
                if(bx<0||by<0||bx>=cur.lvl.walls.columns||by>=cur.lvl.walls.rows)
                    continue;

                if(bb_get(&cur.lvl.walls,by,bx) || bb_get(&cur.lvl.boxes,by,bx))
                    continue;
            }

            if(nodes_count>=MAX_STATES)
                continue;

            int nidx=nodes_count++;
            copy_level_shallow(&nodes[nidx].lvl,&cur.lvl);
            if(next_is_box){
                int bx=nx+DX[d],by=ny+DY[d];
                bb_clear(&nodes[nidx].lvl.boxes,ny,nx); bb_set(&nodes[nidx].lvl.boxes,by,bx);
            }

            bb_clear(&nodes[nidx].lvl.player,py,px); bb_set(&nodes[nidx].lvl.player,ny,nx);

            if (simple_deadlock(&nodes[nidx].lvl)){
                free_level_shallow(&nodes[nidx].lvl); nodes_count--; continue;
            }

            nodes[nidx].g=cur.g+1;
            nodes[nidx].h=heuristic(&nodes[nidx].lvl);
            nodes[nidx].f=nodes[nidx].g+nodes[nidx].h;
            nodes[nidx].parent=cur_idx;
            nodes[nidx].move=DIR_CH[d];

            uint64_t k=hash_state(&nodes[nidx].lvl);
            if(!hs_add(visited,k)) {
                free_level_shallow(&nodes[nidx].lvl);
                nodes_count--;
                continue;
            }

            heap_push(open,nidx);
        }
    }

    char* path=NULL;
    if(found_idx!=-1){
        int len=0;
        for(int i=found_idx;i!=0;i=nodes[i].parent) len++;
        path=malloc(len+1);
        int pos=len-1;
        for(int i=found_idx;i!=0;i=nodes[i].parent)
            path[pos--]=nodes[i].move;

        path[len]='\0';
    }

    for(int i=0;i<nodes_count;i++)
        free_level_shallow(&nodes[i].lvl);

    free(nodes);
    heap_free(open);
    hs_free(visited);
    return path;
}