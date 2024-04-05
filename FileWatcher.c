#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>

#define EXIT_SUCCESS 0
#define EXIT_ERR_TOO_FEW_ARGS 1
#define EXIT_ERR_NO_INIT 2
#define EXIT_ERR_NO_WATCH 3
#define EXIT_ERR_NO_READ_EVENT 4

int main(int argc, char** argv)
{
  char *basePath, *token, *eventMessage;
  char buffer[4096];
  ssize_t readSize;
  int eventQueue, wd;
  const struct inotify_event *event;
  uint32_t bitMask = IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_OPEN | IN_CLOSE_NOWRITE;

  if(argc < 2) {
    fprintf(stderr, "I need a filepath\n");
    exit(EXIT_ERR_TOO_FEW_ARGS);
  }

  basePath = (char *)malloc(sizeof(char) * strlen(argv[1]));
  strcpy(basePath, argv[1]);

  token = strtok(basePath, "/");

  while(token != NULL) {
    basePath = token;
    token = strtok(NULL, "/");
  }

  printf("Watching directory %s\n", basePath);

  eventQueue = inotify_init();
  if(eventQueue == -1) {
    fprintf(stderr, "Couldn't initialize inotify\n");
    exit(EXIT_ERR_NO_INIT);
  }

  wd = inotify_add_watch(eventQueue, argv[1], bitMask);
  if(wd == -1) {
    fprintf(stderr, "Couldn't watch the directory %s\n", basePath);
    exit(EXIT_ERR_NO_WATCH);
  }

  while(1) {
    printf("Waiting for events!\n");

    readSize = read(eventQueue, buffer, sizeof(buffer));
    if(readSize == -1) {
      fprintf(stderr, "Couldn't read the event queue\n");
      exit(EXIT_ERR_NO_READ_EVENT);
    }
    for(char *ptr = buffer; ptr < buffer + readSize; ptr += sizeof(struct inotify_event) + event->len) {

      event = (const struct inotify_event *)ptr;
      eventMessage = NULL;

      if(event->mask & IN_CREATE)
        eventMessage = "File/directory was created in watched directory\n";
      if(event->mask & IN_OPEN)
        eventMessage = "Watched directory was opened\n";
      if(event->mask & IN_DELETE)
        eventMessage = "File/directory deleted from watched directory\n";
      if(event->mask & IN_DELETE_SELF)
        eventMessage = "Watched directory was deleted\n";
      if(event->mask & IN_MOVE_SELF)
        eventMessage = "Watched directory was moved\n";
      if(event->mask & IN_CLOSE_NOWRITE)
        eventMessage = "Watched directory was closed\n";

      if(eventMessage == NULL)
        continue;

      printf("%s\n", eventMessage);
    }
  }

  printf("Stopped watching...\n");
  close(eventQueue);
  exit(EXIT_SUCCESS);
}
