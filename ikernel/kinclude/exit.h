#ifndef EXIT_H_
#define EXIT_H_

struct iwait_opts {
	struct itask       	*wo_target;
	int                	 wo_stat;
	int                	 wo_flags;

	iwait_queue_entry_t	 child_wait;
	int                	 notask_error;
};

#endif // EXIT_H_
