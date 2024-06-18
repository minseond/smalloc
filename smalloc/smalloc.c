/*

mmap을 이용하여 메모리 관리하기

- smalloc은 firstfit으로 구현하였습니다.

- 메모리를 할당할 때 사용하고 남은 공간이 다음 헤더의 크기보다 작거나 같은 경우
spilt을 하지 않았습니다.
ex. 4072의 공간에 4070을 할당하면 2의 메모리가 남으므로 split하지 않음
4048은 24가 남으므로 이 공간을 사용할 수 없기에 split하지 않음

- 메모리 병합은 mmap으로 생성한 메모리 청크의 크기로 했으며 다른 mmap으로 생성된 메모리 청크간의 병합은 일어나지 않습니다.
예를 들어 mmap으로 크기가 4096, 8192인 메모리 청크를 만든 상태에서 메모리 병합을 위해 smcoalesce()를 호출하면
4096과 인접한 8192의 영역은 병합 시키지 않으나 각 청크내에서 반환된 인접한 메모리들은 병합시킵니다.
이렇게 구현한 이유는 가상메모리를 병합하는 과정에서 메모리 청크보다 커진 경우가 생겼는데 이 메모리에 접근하는 도중
segmentation fault를 겪었고 메모리 청크가 물리 메모리에 매핑될 경우 연속적이지 않을 수 있기 때문에 이러한 문제가 생기는 것이라 예상했습니다.
프로그램에서 보여지는 메모리는 실제 메모리가 아닌 가상 메모리이므로 병합을 메모리 청크끼리만 결합하여 프로그램을 실행하니 잘 작동되는 것을
확인할 수 있었습니다.
*/
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "smalloc.h"
#include <string.h>

smheader_ptr smlist = 0x0 ;//첫 번째 data header 
size_t psizeArr[10000] ={0};// mmap으로 할당되는 메모리 크기 저장하는 배열 -> 병합에 사옹
int pcnt =0; // mmap호출 횟수 저장

//smalloc 함수 
void * smalloc (size_t s) 
{//s만큼 메모리 할당
   //mmap은 pagesize의 배수로 메모리를 할당함.
   int psize =0;//mmap 쓸 경우 할당되는 공간크기
   if(((s+sizeof(smheader))%getpagesize()) != 0 ) 
      psize = getpagesize() *(((s+sizeof(smheader))/getpagesize()) + 1);
   else 
      psize = getpagesize() *((s+sizeof(smheader))/getpagesize());
   smheader_ptr newM;//mmap할 경우 쓸 변수
   if(smlist == NULL){//mmap 처음 실행
      newM = (smheader_ptr)mmap(NULL, s +sizeof(smheader), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);// 메모리 할당
      psizeArr[pcnt++] = psize; //mmap 크기 저장

      if(newM == MAP_FAILED){
         perror("mmap failed\n");
         abort();
      }
      newM->size = s; //크기 저장
      newM->used = 1; //사용중으로 변경 
      if(s>= psize-sizeof(smheader) && s<= psize+sizeof(smheader)){
        newM->next =NULL;
      }
      else{
        newM->next = (void*)newM + s + sizeof(smheader);// split
        newM->next->size = psize - s - sizeof(smheader) *2; 
        newM->next->used =0;
        newM->next->next =NULL;
      }
      smlist = newM;      
      
      return (void*)((void*)newM+sizeof(smheader));
   }

   smheader_ptr currentptr= smlist;
   while(currentptr != NULL){
    if(!currentptr->used && (currentptr->size >= s)){//적합한 region 있는 경우
        currentptr->used = 1;
        smheader_ptr oldptr = currentptr->next;//split되기 전 가리키는 header
        if(s>= currentptr->size-sizeof(smheader) && s<= currentptr->size){
            currentptr->next =oldptr;
        }
        else{
            currentptr->next =((void*)currentptr + sizeof(smheader) +s);//split
        // currentptr->next =((void*)currentptr + 1090);//split
            currentptr->next->used = 0;
            currentptr->next->size = currentptr->size -s -sizeof(smheader);
            currentptr->next->next = oldptr;
        }
        
        currentptr->size = s;
        return (void*)((void*)currentptr + sizeof(smheader));
        }
        if(currentptr->next == NULL) break;
        currentptr = currentptr->next;
   }

    //적합한 공간이 없는 경우 메모리할당
    newM = (smheader_ptr)mmap(NULL, s+sizeof(smheader), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    psizeArr[pcnt++] = psize;
    if(newM == MAP_FAILED){
        perror("mmap failed\n");
        return NULL;
    }
    newM->size = s;
    newM->used = 1;
    if(s>= psize-sizeof(smheader) && s<= psize+sizeof(smheader)){
        newM->next =NULL;
      }
    else{
        newM->next = (void*)newM + s + sizeof(smheader);// split 
        newM->next->size =psize- s - sizeof(smheader)*2; 
        newM->next->used =0;
        newM->next->next =NULL;
    }
    currentptr->next= newM;

    return (void*)((void*)newM+sizeof(smheader));
}

void *smalloc_mode(size_t s, smmode m) {
    if(m == firstfit) return smalloc(s);
    smheader_ptr selected_ptr = NULL;
    size_t selected_size = (m == bestfit) ? SIZE_MAX : 0;
    
    // 데이터 영역 탐색
    smheader_ptr currentptr = smlist;
    while (currentptr != NULL) {//탐색
    // 현재 영역이 사용되지 않았고, 요청 크기보다 크거나 같은 경우
        if (!currentptr->used && currentptr->size >= s) {
            //bestfit과 worstfit에 적합한 메모리 찾기
            if ((m == bestfit && currentptr->size < selected_size) ||
                (m == worstfit && currentptr->size > selected_size)) {
                selected_size = currentptr->size;
                selected_ptr = currentptr;
            }
        }
        currentptr = currentptr->next;
    }
    if(selected_ptr == NULL) return smalloc(s); //적합한 공간 없으면 새로 할당
    
    selected_ptr->used = 1;//적합한 공간 있는 경우
    smheader_ptr old_next = selected_ptr->next;
    if(selected_ptr->size > s + sizeof(smheader)){//분할하는 경우
        selected_ptr->next = (void*)selected_ptr + sizeof(smheader) + s;
        selected_ptr->next->size = selected_ptr->size - s - sizeof(smheader);
        selected_ptr->next->used = 0;
        selected_ptr->next->next = old_next;
    }//분할 안하는 경우는 그대로 
    selected_ptr->size = s;
    return (void*)((void*)selected_ptr + sizeof(smheader));
}

void sfree(void *p) {
    //하나만 있는 경우 temp == smlist
    //p가 마지막인 경우 currentptr->next = null
    smheader_ptr temp = (smheader_ptr)(p - sizeof(smheader));

    smheader_ptr currentptr = smlist;
    while (currentptr != NULL){
        if (currentptr == temp){
            currentptr->used = 0; 
            return;
        }
        currentptr = currentptr->next;
    }
    printf("%p doesn't exist\n", p);
    abort();
}

void * srealloc(void *p, size_t s) {
    smheader_ptr currentptr = smlist;
    smheader_ptr searchptr = p-sizeof(smheader);

    while(currentptr != NULL){//p가 있는지 확인 후 없으면 종료
        if(currentptr == searchptr) break;
        currentptr = currentptr->next;
    }
    if(currentptr == NULL){
        printf("%p doesn't exist\n", p);
        abort();
    }

    size_t currentSize = searchptr->size;
    if (s == currentSize) {
        // 크기 변하지 않는 경우
        return p;
    } 
    else if (s < currentSize) {
        // 요청된 크기가 현재 크기보다 작은 경우 분할
        size_t remaining = currentSize - s - sizeof(smheader);
        if (remaining >= sizeof(smheader)) {
            // 분할 가능
            smheader_ptr newHeader = (smheader_ptr)((void*)searchptr + sizeof(smheader) + s);
            newHeader->size = remaining;
            newHeader->used = 0;
            newHeader->next = searchptr->next;

            searchptr->size = s;
            searchptr->next = newHeader;
        }
        return p;
    } 
    else {
        // 요청된 크기가 현재 크기보다 큰 경우
        void* newPtr = smalloc(s);
        smheader_ptr temp = newPtr - sizeof(smheader);

        if (newPtr == NULL) {
            return NULL; // 메모리 할당 실패
        }
        temp->size = s;
        temp->used = 1;
        // temp->next = searchptr->next; //할당받은 메모리는 청크에 이미 연결되어 있으므로 바꾸면 안됨
        sfree(p); // 기존 메모리 블록 해제

        return newPtr;
    }
}

void smcoalesce() {
    smheader_ptr current = smlist;
    for(int i=0; i<pcnt; i++){//물리메모리가 연속된 공간에서만 병합
        size_t unsize =0;
        size_t usingsize =0;

        while( unsize + usingsize !=psizeArr[i] && current !=NULL){
            if(current->used) usingsize += current->size + sizeof(smheader);
            else unsize += current->size + sizeof(smheader);
            
            if(usingsize + unsize == psizeArr[i]) {
                current = current->next;
                break;
            }
            if(!current->used && !current->next->used){//병합
                unsize -= (current->size +sizeof(smheader));
                current->size += current->next->size + sizeof(smheader);
                current->next =current->next->next; //current 다시 확인
            }
            else current = current->next;
        }
    }
}

void smdump () 
{
   smheader_ptr itr ;
   //data region
   printf("==================== used memory slots ====================\n") ;
   int i = 0 ;
   for (itr = smlist ; itr != 0x0 ; itr = itr->next) {
      if (itr->used == 0)
         continue ;

      printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(smheader), (int) itr->size) ;

      char * s = ((char *) itr) + sizeof(smheader) ;
      for (size_t j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++)  {
         printf("%02x ", s[j]) ;
      }
      printf("\n") ;
      i++ ;
   }
   printf("\n") ;

   printf("==================== unused memory slots ====================\n") ;
   i = 0 ;
   for (itr = smlist ; itr != 0x0 ; itr = itr->next, i++) {
      if (itr->used == 1)
         continue ;

      printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(smheader), (int) itr->size) ;

      char * s = ((char *) itr) + sizeof(smheader) ;
      for (size_t j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) {
         printf("%02x ", s[j]) ;
      }
      printf("\n") ;
      i++ ;
   }
   printf("\n") ;
}