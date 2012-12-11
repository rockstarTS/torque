#include "license_pbs.h" /* See here for the software license */
#include <pbs_config.h>   /* the master config generated by configure */
#include "queue_recycler.h"

#include "queue.h" /* pbs_queue, all_queues */
#include "queue_func.h"


#include "utils.h"
#include "threadpool.h"

extern queue_recycler q_recycler;
extern int LOGLEVEL;


void initialize_queue_recycler()

  {
  pthread_mutexattr_t t_attr;
  pthread_mutexattr_init(&t_attr);
  pthread_mutexattr_settype(&t_attr, PTHREAD_MUTEX_NORMAL);
  q_recycler.next_id = 0;
  q_recycler.max_id = 10;
  initialize_allques_array(&q_recycler.queues);
  q_recycler.iter = -1;

  q_recycler.mutex = calloc(1, sizeof(pthread_mutex_t));
  pthread_mutex_init(q_recycler.mutex,&t_attr);
  } /* END initialize_recycler() */




pbs_queue *next_queue_from_recycler(
    
  struct all_queues *aq,
  int             *iter)

  {
  pbs_queue *pq;

  pthread_mutex_lock(aq->allques_mutex);
  pq = next_thing(aq->ra, iter);
  if (pq != NULL)
    lock_queue(pq, __func__, (char *)NULL, LOGLEVEL);
  pthread_mutex_unlock(aq->allques_mutex);

  return(pq);
  } /* END next_queue_from_recycler() */




void *remove_some_recycle_queues(

  void *vp)

  {
  int        iter = -1;
  pbs_queue *pq;

  pthread_mutex_lock(q_recycler.mutex);

  pq = next_queue_from_recycler(&q_recycler.queues,&iter);

  if (pq == NULL)
    return NULL;

  remove_queue(&q_recycler.queues,pq);
  unlock_queue(pq, __func__, (char *)NULL, LOGLEVEL);
  free(pq->qu_mutex);
  free_alljobs_array(pq->qu_jobs);
  free_alljobs_array(pq->qu_jobs_array_sum);
  memset(pq, 254, sizeof(pbs_queue));
  free(pq);

  pthread_mutex_unlock(q_recycler.mutex);

  return(NULL);
  } /* END remove_some_recycle_queues() */




int insert_into_queue_recycler(

  pbs_queue *pq)

  {
  int              rc;

  pthread_mutex_lock(q_recycler.mutex);

  memset(pq->qu_qs.qu_name, 0, sizeof(pq->qu_qs.qu_name));
  sprintf(pq->qu_qs.qu_name,"%d",q_recycler.next_id);
  pq->q_being_recycled = TRUE;

  if (q_recycler.queues.ra->num >= MAX_RECYCLE_QUEUES)
    {
    enqueue_threadpool_request(remove_some_recycle_queues,NULL);
    }

  rc = insert_queue(&q_recycler.queues,pq);

  update_queue_recycler_next_id();

  pthread_mutex_unlock(q_recycler.mutex);

  return(rc);
  } /* END insert_into_queue_recycler() */




void update_queue_recycler_next_id()

  {
  if (q_recycler.next_id >= q_recycler.max_id)
    q_recycler.next_id = 0;
  else
    q_recycler.next_id++;
  } /* END update_queue_recycler_next_id() */

