#include <iostream>
#include <omp.h>
#include <vector>
#ifndef N
#define N 5
#endif
#ifndef FS
#define FS 38
#endif

struct node {
   int data;
   int fibdata;
   struct node* next;
};

int fib(int n) {
   int x, y;
   if (n < 2) {
      return (n);
   } else {
      x = fib(n - 1);
      y = fib(n - 2);
	  return (x + y);
   }
}

void processwork(struct node* p) 
{
   int n;
   n = p->data;
   p->fibdata = fib(n);
}

struct node* init_list(struct node* p) {
    int i;
    struct node* head = NULL;
    struct node* temp = NULL;
    
    head = (struct node*)malloc(sizeof(struct node));
    p = head;
    p->data = FS;
    p->fibdata = 0;
    for (i=0; i< N; i++) {
       temp  =  (struct node*)malloc(sizeof(struct node));
       p->next = temp;
       p = temp;
       p->data = FS + i + 1;
       p->fibdata = i+1;
    }
    p->next = NULL;
    return head;
}

int main(int argc, char *argv[]) {
     double start, end;
     struct node *p=NULL;
     struct node *temp=NULL;
     struct node *head=NULL;
     
	 std::printf("Process linked list\n");
     std::printf("  Each linked list node will be processed by function 'processwork()'\n");
     std::printf("  Each ll node will compute %d fibonacci numbers beginning with %d\n",N,FS);      
 
     p = init_list(p);
     head = p;

     int counter{0};
    while(p != nullptr)
    {
        p = p->next;
        counter++;
    }
    p=head;
    std::vector<struct node*> ps (counter);
    for(int i = 0; i < counter; i++){
        ps[i]=p;
        p=p->next;
    }
    start = omp_get_wtime();

    #pragma omp parallel for
    for(int i=0; i<counter; i++){
        processwork(ps[i]);
    }
    

     end = omp_get_wtime();
     p = head;
	 while (p != NULL) {
        std::printf("%d : %d\n",p->data, p->fibdata);
        temp = p->next;
        free (p);
        p = temp;
     }  
	 free (p);

     std::printf("Compute Time: %f seconds\n", end - start);

     return 0;
}
