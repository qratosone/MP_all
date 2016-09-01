/** @file log.c */
#include <stdlib.h>
#include <string.h>
#include "log.h"

/**
 * Initializes the log.
 *
 * You may assuem that:
 * - This function will only be called once per instance of log_t.
 * - This function will be the first function called per instance of log_t.
 * - All pointers will be valid, non-NULL pointer.
 *
 * @returns
 *   The initialized log structure.
 */
void log_init(log_t *l) {
    l->head=NULL;
}

/**
 * Frees all internal memory associated with the log.
 *
 * You may assume that:
 * - This function will be called once per instance of log_t.
 * - This funciton will be the last function called per instance of log_t.
 * - All pointers will be valid, non-NULL pointer.
 *
 * @param l
 *    Pointer to the log data structure to be destoryed.
 */
void log_destroy(log_t* l) {
    if(!l->head){
        return ;
    }
    node* this=l->head;
    while(this!=NULL){
        node* temp=this;
        this=this->next;
        free(temp);
    }
    l->head=NULL;
}

/**
 * Push an item to the log stack.
 *
 *
 * You may assume that:
* - All pointers will be valid, non-NULL pointer.
*
 * @param l
 *    Pointer to the log data structure.
 * @param item
 *    Pointer to a string to be added to the log.
 */
void log_push(log_t* l, const char *item) {
    if(l->head==NULL){
        l->head=(node*)malloc(sizeof(node));
        l->head->next=NULL;
        int length=strlen(item);
        l->head->cmd=(char*)malloc(length+2);
        strcpy(l->head->cmd,item);
        return;
    }
    node* this=l->head;
    while(this!=NULL){
        this=this->next;
    }
    this=(node*)malloc(sizeof(node));
    this->next=NULL;
    int length=strlen(item);
    this->cmd=(char*)malloc(length+2);
    strcpy(this->cmd,item);
}


/**
 * Preforms a newest-to-oldest search of log entries for an entry matching a
 * given prefix.
 *
 * This search starts with the most recent entry in the log and
 * compares each entry to determine if the query is a prefix of the log entry.
 * Upon reaching a match, a pointer to that element is returned.  If no match
 * is found, a NULL pointer is returned.
 *
 *
 * You may assume that:
 * - All pointers will be valid, non-NULL pointer.
 *
 * @param l
 *    Pointer to the log data structure.
 * @param prefix
 *    The prefix to test each entry in the log for a match.
 *
 * @returns
 *    The newest entry in the log whose string matches the specified prefix.
 *    If no strings has the specified prefix, NULL is returned.
 */
char *log_search(log_t* l, const char *prefix) {
    if(l->head==NULL){
        return NULL;
    }
    char* ret=NULL;
    node* this=l->head;
    while(this!=NULL){
        if(!strncmp(this->cmd,prefix,strlen(prefix))){
            ret=this->cmd;
        }
        this=this->next;
    }
    return ret;
}
