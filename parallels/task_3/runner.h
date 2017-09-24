#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>

typedef void (*run_cb_t)(void *);

static inline void timer(char *phase, run_cb_t cb, void *ctx)
{
	printf("%s\n", phase);
	struct timeval start, end;

	assert(gettimeofday(&start, NULL) == 0);

	cb(ctx);

	assert(gettimeofday(&end, NULL) == 0);

	double delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
	printf("%s: %lf s\n\n", phase, delta);
}

void runner_pre(run_cb_t cb, void *ctx)
{
	timer("pre-phase", cb, ctx);
}

#define MAX_NAME_LENGTH 64

void runner_run(run_cb_t cb, void *ctx, const char *name)
{
	const char *phase = "running phase";
	char buf[MAX_NAME_LENGTH] = {0};

	assert(strlen(name) + strlen(phase) + 2 < MAX_NAME_LENGTH); // 2 is for space and '\0'

	strcat(buf, phase);
	if (strlen(name) > 0) {
		strcat(buf, " ");
		strcat(buf, name);
	}

	timer(buf, cb, ctx);
}

void runner_post(run_cb_t cb, void *ctx)
{
	timer("post-phase", cb, ctx);
}
