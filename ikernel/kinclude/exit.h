#ifndef EXIT_H_
#define EXIT_H_

struct iwait_opts {
	enum tid_type		 wo_type;
	int			 wo_flags;
	struct ktid		*wo_ktid;

	int			 wo_stat;

	iwait_queue_entry_t	 child_wait;
	int			 notask_error;
};

extern long __do_wait(struct iwait_opts *wo);

#endif // EXIT_H_
