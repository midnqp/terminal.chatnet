#include <libgen.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "autofree.h"
#include "db.h"
#include "string.h"
#include "util.h"
#include "sio-client.h"

int main(int argc, char *argv[]) {

	atexit(dealloc);

	sioclientinit();

	leveldbinit();

	return 0;
}