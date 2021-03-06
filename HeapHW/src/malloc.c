#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1) 

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allocated memory   */
   struct _block *next;  /* Pointer to the next _block of allocated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *previous = NULL;

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /*  
    * Best Fit
    *
    * best fit searches through the heap in search of
    * the block with the least amount of wasted space
    */
   struct _block *winner = NULL;
   size_t leftover = -1;
   while (curr)                                       // If we are not NULL keep going
   {
      if(curr->free && (curr->size >= size))          // If the spot is large enough:
      {
         if(leftover == -1)                           // Will take the first available
         {
            leftover = curr->size - size;             
            winner = curr;                            // Set winner to best fit
         }
         else if(leftover > (curr->size - size))      // Compare leftover to previous
         {                                            // we want a smaller leftover
            leftover = curr->size - size;
            winner = curr;
         }
      }
      *last = curr;
      curr  = curr->next;
   }

   curr = winner;                                     // Set block to best fit
#endif

#if defined WORST && WORST == 0
   /*  
    * Worst Fit
    *
    * worst fit searches through  the heap in search of
    * the block with the largest amount of wasted space
    */
   struct _block *winner = NULL;
   size_t leftover = -1;
   while (curr)                                       // If we are not NULL keep going
   {
      if(curr->free && (curr->size >= size))          // If the spot is large enough:
      {
         if(leftover == -1)                           // Will take the first available
         {
            leftover = curr->size - size;
            winner = curr;                            // Set winner to best fit
         }
         else if(leftover < (curr->size - size))      // Compare leftover to previous
         {                                            // we want a larger leftover
            leftover = curr->size - size;
            winner = curr;
         }
      }
      *last = curr;
      curr  = curr->next;
   }

   curr = winner;                                     // Set block to worst fit
#endif

#if defined NEXT && NEXT == 0
   /*  
    * Next Fit
    *
    * next fit looks through the entire heap for the first
    * spot starting from the previous malloc pointer if it
    * cant find a spot in the first pass it then  restarts
    * and   searches   until   it   finds   a   spot    or 
    * gets   to   the   previous   pointer   from   before
    */
   if(previous != NULL)
      curr = previous;

   while (curr && !(curr->free && curr->size >= size)) 
   {

      *last = curr;
      curr  = curr->next;
   }
   
   if(!curr)
   {
      curr = heapList;
      while(curr != previous && !(curr->free && curr->size >= size))
      {
         *last = curr;
         curr  = curr->next;
      }
   }

   if(curr == previous)
   {
      curr = NULL;
   }
#endif

   previous = curr;
   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);
   max_heap += sizeof(struct _block) + size;

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{ 
   
   num_requested += size;
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }
      /* Align to multiple of 4 */
      size = ALIGN4(size);

      /* Handle 0 size */
      if (size == 0) 
      {
         return NULL;
      }

      /* Look for free _block */
      struct _block *last = heapList;
      struct _block *next = findFreeBlock(&last, size);

      /* Could not find free _block, so grow heap */
      if (next == NULL) 
      {
         num_grows++;
         next = growHeap(last, size);
      }
      else
      {
         if(num_blocks > 0 ) num_blocks--;
         num_reuses++;
      }

      /* Could not find free _block or grow heap, so just return NULL */
      if (next == NULL) 
      {
         return NULL;
      }
      

      /* Mark _block as in use */
      next->free = false;
   
   /* Return data address associated with _block */
   num_mallocs++;
   return BLOCK_DATA(next);
}

/*
 * \breif calloc
 * 
 * calls malloc based on number and size of elements
 * then   initilizes   all   the  space  to  nothing
 * 
 * \param memb number of of elements
 * \param size size of each element
 * 
 * \returns the ptr created
 */ 
void* calloc(size_t memb, size_t size)
{
   void *ptr;
   ptr = malloc((memb * size));
   memset(ptr, '\0', (memb * size));
   return ptr;
}

/*
 * \breif realloc
 * 
 * calls malloc with a new size passed in then
 * copys the old data into the new pointer
 * then frees the old pointer
 * 
 * \param ptr old pointer previously malloc
 * \param size size of the array to be malloced
 * 
 * \returns the ptr created
 */ 
void* realloc(void* ptr, size_t size)
{
   void *new;
   new = malloc(size);
   memcpy(new, ptr, size);
   free(ptr);
   return new;
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   num_frees++;
   num_blocks++;
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
