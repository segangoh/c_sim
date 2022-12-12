#include <stdio.h>
#include <stdlib.h>

// ���� ��
#define filename "swim.trace"


struct mycache{ // ĳ�� ����ü 
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

// �ùķ��̼� �Լ�: ��Ʈ��, ��� ��, ��� ����� �μ��� �޴´�
void c_sim(int num_set, int num_block, int block_size){
    /* �������� �� �ʱ�ȭ */
    l_total=l_miss=s_total=s_miss=0;

    char mode;
    int address;
    int num = num_set*num_block;
    FILE *fp = NULL;
    
    dp = (struct mycache*)calloc(num, sizeof(struct mycache));
    
    fp = fopen(filename, "r");
	// ���Ϸκ��� mode�� �ּҸ� �о� �� �Լ��� ����
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
    
    set = (address/block_size) % num_set;  // �ش� �ּ��� ��Ʈ ���ϱ�
    
    // set �˻��� ���� hit, miss�� ����
    for(i=0; i<num_block; i++){
        p = &dp[set*num_block + i];	
        // val bit�� 1�̰� tg���� ��ġ�� ��
        if(p->val==1 && p->tg==(address/block_size)/num_set){
            p->time = cnttime;
            return;
        }
		// ���� ���� �� ��
        else if(p->val==0){
            if(i < capacity)
                capacity = i;
        }
    }
    // �ش�Ǵ� ���� ������ miss
    l_miss++;
    // set�� ���� ã�� ��
    if(capacity==10){
        lru = LRUcnt(set, num_block);
        p = &dp[set*num_block + lru];
        
		// dirty bit�� 1�� �� �޸� ����
        if(p->dirty)
            m_write++;
        
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 0;   //�� ���� dirty bit 0
        
    }
	// set�� �ڸ��� ���� ��
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

    set = (address/block_size) % num_set;  // �ش� �ּ��� ��Ʈ ���ϱ�
    
    // set �˻��� ���� hit, miss�� ����
    for(i=0; i<num_block; i++){
        p = &dp[set*num_block + i];
        // val bit�� 1�̰� tg���� ��ġ�� ��
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
    // �ش�Ǵ� ���� ������ miss
    s_miss++;
    // set�� ���� ã�� ��
    if(capacity==10){
        lru = LRUcnt(set, num_block);
        p = &dp[set*num_block + lru];
        
        // dirty bit�� 1�� �� �޸� ����
        if(p->dirty)
            m_write++;
        
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 1;   //�� ������ dirty bit 1
    }
   // set�� �ڸ��� ���� ��
    else{
        p = &dp[set*num_block + capacity];
        p->val = 1;
        p->time = cnttime;
        p->tg = (address/block_size)/num_set;
        p->dirty = 1;
    }
}

// lru ���� �Լ�
int LRUcnt(int set, int num_block){
    int i, temp=0;  
    int minimum = cnttime+1, las=0;
    
    // cnttime ���� �ּ��� ���� ã�� ����
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
    int num_set[4] = {1, 2, 4, 8}; //��Ʈ ��
    int block_size[2] = {4, 16};    // ���� ����Ʈ ��
    int num_block[3] = {1, 2, 4};  //���� ��
    int i,j;
 	int input=0;
	for(i=0; i<2; i++){
		for(j=0; j<3; j++){
			c_sim(num_set[1], block_size[i], num_block[j]);
		}
	}
    return 0;
}
