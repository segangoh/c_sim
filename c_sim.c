#include <stdio.h>
#include <stdlib.h>

// 파일 명
#define filename "swim.trace"


struct mycache{ // 캐시 구조체 
    int tg;	
    int val;	
    int time;	
    int dirty;	
};

int l_total, l_miss, s_total, s_miss, m_write;
int cnttime;

struct mycache *dp;

void c_sim(int num_set, int num_block, int block_size);
void load_data(int address, int num_set, int num_block, int block_size);
void store_data(int address, int num_set, int num_block, int block_size);
int LRUcnt(int set, int num_block);

// 시뮬레이션 함수: 세트수, 블록 수, 블록 사이즈를 인수로 받는다
void c_sim(int num_set, int num_block, int block_size){
    /* 전역변수 값 초기화 */
    l_total=l_miss=s_total=s_miss=0;

    char mode;
    int address;
    int num = num_set*num_block;
    FILE *fp = NULL;
    
    dp = (struct mycache*)calloc(num, sizeof(struct mycache));
    
    fp = fopen(filename, "r");
	// 파일로부터 mode와 주소를 읽어 각 함수에 전달
    while(!feof(fp)){
        fscanf(fp, "%c %x %*d\n", &mode, &address);
        
        switch(mode){
            case 'l' :
                load_data(address, num_set, num_block, block_size);
                l_total++;
                break;
            case 's' :
                store_data(address, num_set, num_block, block_size);
                s_total++;
                break;
        }
        cnttime++;
    }
    
    free(dp);

    int c_size = num_set*num_block*block_size;
    printf("num of set | num of block | block size | cash size\n");
    printf("%6d%14d%14d%14d\n", num_set, num_block, block_size, c_size);
    printf("Total loads | Total stores | Load hits | Load misses | Store hits | Store misses\n");
    printf("%6d%14d%14d%14d%14d%14d\n", l_total, s_total, l_total-l_miss, l_miss, s_total-s_miss, s_miss);
    fclose(fp);
}

void load_data(int address, int num_set, int num_block, int block_size){
    int set;
    int i, lru=0, capacity=10;
    struct mycache *p;
    
    set = (address/block_size) % num_set;  // 해당 주소의 세트 구하기
    
    // set 검색을 통해 hit, miss를 결정
    for(i=0; i<num_block; i++){
        p = &dp[set*num_block + i];	
        // val bit이 1이고 tg값이 일치할 때
        if(p->val==1 && p->tg==(address/block_size)/num_set){
            p->time = cnttime;
            return;
        }
		// 블럭이 새로 들어갈 때
        else if(p->val==0){
            if(i < capacity)
                capacity = i;
        }
    }
    // 해당되는 블럭이 없으면 miss
    l_miss++;
    // set가 가득 찾을 때
    if(capacity==10){
        lru = LRUcnt(set, num_block);
        p = &dp[set*num_block + lru];
        
		// dirty bit이 1일 때 메모리 쓰기
        if(p->dirty)
            m_write++;
        
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 0;   //새 블럭은 dirty bit 0
        
    }
	// set에 자리가 남을 때
    else{
        p = &dp[set*num_block + capacity];
        
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 0;
    }
}

void store_data(int address, int num_set, int num_block, int block_size){
    int set;
    int i, lru=0, capacity=10;
    struct mycache *p;

    set = (address/block_size) % num_set;  // 해당 주소의 세트 구하기
    
    // set 검색을 통해 hit, miss를 결정
    for(i=0; i<num_block; i++){
        p = &dp[set*num_block + i];
        // val bit이 1이고 tg값이 일치할 때
        if(p->val==1 && p->tg==(address/block_size)/num_set){
            p->time = cnttime;
            p->dirty = 1;
            return;
        }
        else if(p->val==0){
            if(i < capacity)
                capacity = i;
        }
    }
    // 해당되는 블럭이 없으면 miss
    s_miss++;
    // set가 가득 찾을 때
    if(capacity==10){
        lru = LRUcnt(set, num_block);
        p = &dp[set*num_block + lru];
        
        // dirty bit이 1일 때 메모리 쓰기
        if(p->dirty)
            m_write++;
        
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 1;   //새 블럭또한 dirty bit 1
    }
   // set에 자리가 남을 때
    else{
        p = &dp[set*num_block + capacity];
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 1;
    }
}

// lru 구현 함수
int LRUcnt(int set, int num_block){
    int i, temp=0;  
    int minimum = cnttime+1, las=0;
    
    // cnttime 값이 최소인 블럭을 찾아 리턴
    for(i=0; i<num_block; i++){
        temp = dp[set*num_block + i].time;
        if(minimum > temp){
            minimum = temp;
            las = i;
        }
    }
    return las;
}

int main(){
    int num_set[4] = {1, 2, 4, 8}; //세트 수
    int block_size[2] = {4, 16};    // 블럭의 바이트 수
    int num_block[3] = {1, 2, 4};  //블럭의 수
    int i,j;
 	int input=0;
	for(i=0; i<2; i++){
		for(j=0; j<3; j++){
			c_sim(num_set[1], block_size[i], num_block[j]);
		}
	}
    return 0;
}
