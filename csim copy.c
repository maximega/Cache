#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cachelab.h"
#define MAXLINE 500
//maximega@bu.edu

int s = 0;
int E = 0;
int b = 0;
char * pathname;
int opt;
int hit_count;
int miss_count;
int eviction_count;
int access_count = 0;
int * cache_ptr;


void process_line(char * mem_addr);
void readfile();
void cmd_line(int argc, char * argv[]);
void check_inCache(char instr, unsigned int tag, unsigned int set_index);
void evict (unsigned int set_index, unsigned int tag);

typedef struct
{
  unsigned int value;
  int access;
}cacheBlock;

int main(int argc, char * argv[])
{
  cmd_line(argc, argv);
  readfile();
  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}

void process_line(char * mem_addr)
{
  
  char instr;
  unsigned int size;
  char * addr_array = malloc(16);
  instr = mem_addr[1];
  for(int i = 3 ;; i++)
  {
    if(mem_addr[i] != ',')
	    {
	      addr_array[i-3] = mem_addr[i];
	    }
    else if(mem_addr[i] == ',')
	    {
	      size = (unsigned int) (mem_addr[i+1]) - 48;
	      addr_array[i-3] = '\0';
	      break;
	    }
    }

    
    unsigned int addr = (unsigned int)strtoul(addr_array, NULL, 16);
    unsigned int block_offset = addr << (32 - b);
    block_offset = block_offset >> (32 - b);
    //printf("This is my block offset: %u", block_offset);
    unsigned int set_index = addr << (32 - (s + b));
    set_index = set_index >> (32 - s);
    //printf("This is my set index: %u", set_index);
    unsigned int tag = addr >> (s+b);
    //printf("This is my tag: %u", tag);

    if (instr == 'L')
    {
      check_inCache(instr, tag, set_index);
    }
    else if (instr == 'S')
    {
      check_inCache(instr, tag, set_index);
    }
    else if (instr == 'M')
    {
      check_inCache('L', tag, set_index);
      check_inCache('S', tag, set_index);
    } 
}



void readfile()
{
  FILE * fileptr;
  char * address = malloc(MAXLINE);
  if ((fileptr = fopen(pathname, "r")) == NULL)
    {
      perror("Open Error");
      exit(-1);
    }
  
  while ((fgets(address, MAXLINE, fileptr)) != NULL)
  {
    if(address[0] == ' ')
	  {
      fprintf(stdout, "\n%s", address);
      fflush(stdout);
	    process_line(address);
	  }
  }  
}

void cmd_line(int argc, char * argv[])
{  
  while((opt = getopt(argc, argv, "s:E:b:t:")) != -1)
    switch(opt)
    {
    case 's':
      s = atoi(optarg); //Set this to whatever is passed with s
      break;
    case 'E':
      E = atoi(optarg); //Set this to whatever is passed with E
      break;
    case 'b':
      b = atoi(optarg); //Set this to whatever is passed with b
      break;
    case 't':
      pathname = optarg; //Set this to whatever file is passed with t
      break;
    default:
      abort();
    }
    int num_blocks = (1 << s) * E;
    cache_ptr = calloc(num_blocks, sizeof(cacheBlock));
    memset(cache_ptr, -1, num_blocks);
}

void check_inCache(char instr, unsigned int tag, unsigned int set_index)
{
  int * j = cache_ptr + (set_index * 2 * E);
  int has_hit = 0;
  for (j; j != cache_ptr + ((set_index + 1) * 2 * E); j+=2)
  {
    if (j[0] == tag)
      {
        has_hit = 1;
        hit_count++;
        j[1] = access_count++;
        break; 
      }
  }
  if (has_hit == 0)
    miss_count++;
  has_hit = 0;
  for (j = cache_ptr + (set_index * 2 * E); j != cache_ptr + ((set_index + 1) * 2 * E); j+=2)
  {
    if (instr == 'L')
    {
      break;
    }
    if (instr == 'S')
    {  
        if (j[0] == (unsigned int) -1)
        {
          has_hit = 1;
          j[0] = tag;
          j[1] = access_count++;
          break;
        }
    }
  }
  if (has_hit == 0 && instr == 'S')
  {
    eviction_count++;
    evict(set_index, tag);
  }
}


void evict(unsigned int set_index, unsigned int tag)
{
  int * j;
  int min;
  int * col;
  for (j = cache_ptr + (set_index * 2 * E); j != cache_ptr + ((set_index + 1) * 2 * E); j+=2)
  {
    if (j == cache_ptr + (set_index * 2 * E))
    {
      min = j[1];
      col = j;
    }
    else if (min > j[1])
    {
      min = j[1];
      col = j;
    }
  }
  col[0] = tag;
  col[1] = access_count++;
}




