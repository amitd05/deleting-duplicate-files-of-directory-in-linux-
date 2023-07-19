#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

struct hashtable_entry
{
  unsigned char hash[20];
  char *filepath;
  struct hashtable_entry *next;
};

struct hashtable {
  struct hashtable_entry *head;
};

struct hashtable *hashtable_create() {
  struct hashtable *hashtable = malloc(sizeof(struct hashtable));
  hashtable->head = NULL;
  return hashtable;
}

void hashtable_insert(struct hashtable *hashtable, unsigned char hash[], char *filepath) {
  struct hashtable_entry *entry = malloc(sizeof(struct hashtable_entry));
  memcpy(entry->hash, hash, sizeof(unsigned char) * 20);
  entry->filepath = filepath;
  entry->next = hashtable->head;
  hashtable->head = entry;
}

int hashtable_count(struct hashtable *hashtable, unsigned char hash[]) {
  int count = 0;
  struct hashtable_entry *entry;
  for (entry = hashtable->head; entry != NULL; entry = entry->next) {
    if (memcmp(entry->hash, hash, 20) == 0) {
      count++;
    }
  }
  return count;
}

void hashtable_free(struct hashtable *hashtable) {
  struct hashtable_entry *entry, *next;
  for (entry = hashtable->head; entry != NULL;) {
    next = entry->next;
    free(entry->filepath);
    free(entry);
    entry = next;
  }
  free(hashtable);
}

void file_hash(const char *filepath, unsigned char hash[]) {
  FILE *file = fopen(filepath,"rb");
  if (file == NULL) {
    return;
  }

  int bytes_read;
  unsigned char buffer[1024];
  for (int i = 0; i < 20; i++) {
    bytes_read = fread(hash + i, 1, 1, file);
    if (bytes_read == 0) {
      break;
    }
  }

  fclose(file);
}


int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
    exit(1);
  }

  char *directory = argv[1];
  struct hashtable *hashtable = hashtable_create();
  DIR *dir = opendir(directory);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      continue;
    }

    char *filepath = malloc(strlen(directory) + strlen(entry->d_name) + 2);
    strcpy(filepath, directory);
    strcat(filepath, "/");
    strcat(filepath, entry->d_name);

    unsigned char hash[20];
    file_hash(filepath, hash);
    hashtable_insert(hashtable, hash, filepath);
  }
  closedir(dir);

  for (struct hashtable_entry *entry = hashtable->head; entry != NULL; entry = entry->next) {
    int count = hashtable_count(hashtable, entry->hash);
    if (count > 1) {
      remove(entry->filepath);
    }
  }

  hashtable_free(hashtable);
  return 0;
}

