#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_LISTS 100

#define FATAL(msg) { fprintf(stderr, "FATAL ERROR: %s, exiting.\n", msg); exit(EXIT_FAILURE); }
#define ERROR(msg) { fprintf(stderr, "ERROR: %s\n", msg); }

#define DEFAULT_TIMEOUT 2

typedef struct compute_node_t {
    int id;
    pid_t pid;
    int parent_id; 
    void *socket; // ZeroMQ socket to this node
    struct compute_node_t *prev;
    struct compute_node_t *next;
} compute_node_t;

typedef struct 
{
    int id;
    pid_t pid;
    compute_node_t* arr[MAX_LISTS];
    int list_count;
}control_node_t;

compute_node_t* create_compute_node(int id, int perent_id){
    compute_node_t *node = (compute_node_t *) malloc(sizeof(compute_node_t));
    node->id = id;
    node->pid;
    node->parent_id = perent_id;
    node->socket = NULL; 
    node->prev = NULL;
    node->next = NULL;

    return node;
}

control_node_t* create_control_node() {
    control_node_t* head = (control_node_t*)malloc(sizeof(control_node_t));
    if (!head) {
        perror("Failed to create head node");
        return NULL;
    }
    
    head->id = -1;
    head->list_count = 0;
    for (int i = 0; i < MAX_LISTS; i++) {
        head->arr[i] = NULL;
    }
    
    return head;
}

compute_node_t* find_node(control_node_t* head, int id) {
    if (!head) return NULL;
    
    for (int i = 0; i < head->list_count; i++) {
        compute_node_t* current = head->arr[i];
        while (current) {
            if (current->id == id) {
                return current;
            }
            current = current->next;
        }
    }
    return NULL;
}

compute_node_t* insert_node(compute_node_t* parent, compute_node_t* new_node){
    if (parent->next == NULL){
        parent->next = new_node;
        new_node->prev = parent;
        return new_node;
    }else{
        return NULL;
    }
}

void insert_node_to_head_node(control_node_t* head, compute_node_t* new_node){
    head->arr[head->list_count++] = new_node;
}

void send_command_to_node(compute_node_t *node, const char *command) {
    zmq_msg_t request; 
    zmq_msg_init_size(&request, strlen(command) + 1);
    strcpy(zmq_msg_data(&request), command);

    if (zmq_msg_send(&request, node->socket, 0) == -1) {
        zmq_msg_close(&request);
        if (strcmp(command, "ping") == 0) {
            printf("Ok: 0 // узел %d недоступен\n", node->id);
        } else {
            printf("Error:%d: Node is unavailable\n", node->id);
        }
        return;
    }
    zmq_msg_close(&request);
}

void compute_node_process(int node_id) {

    struct timeval start_time;
    long elapsed_ms = 0;
    bool is_running = false;


    void *zmq_context = zmq_ctx_new();
    void *responder = zmq_socket(zmq_context, ZMQ_PULL);
    char address[50];
    sprintf(address, "tcp://127.0.0.1:%d", 5550 + node_id); 
    int bind_result = zmq_bind(responder, address);
    if (bind_result != 0) {
        printf("Error code: %d", bind_result);
        FATAL("Compute node bind failed");
    }
    printf("Compute Node %d started, listening on %s\n", node_id, address);

    while (1) {


        zmq_msg_t request;
        zmq_msg_init(&request);
        zmq_msg_recv(&request, responder, 0);
        char *command_str = strdup((char *) zmq_msg_data(&request));
        zmq_msg_close(&request);


        char *command = strtok(command_str, " ");
       
        if (strcmp(command, "exec") == 0) {
            char *subcommand = strtok(NULL, " "); 
            if (strcmp(subcommand, "start") == 0) {
                gettimeofday(&start_time, NULL);
                is_running = true;
                printf("Ok:%d\n", node_id);
                } 
            else if (strcmp(subcommand, "stop") == 0) {
                struct timeval current_time;
                gettimeofday(&current_time, NULL);
                
                long seconds = current_time.tv_sec - start_time.tv_sec;
                long micros = current_time.tv_usec - start_time.tv_usec;
                if (micros < 0) {
                    seconds--;
                    micros += 1000000;
                }
                elapsed_ms += seconds * 1000 + micros / 1000;
                
                is_running = false;

                printf("Ok:%d\n", node_id);
            } 
            else if (strcmp(subcommand, "time") == 0) {
                if (is_running) {
                    struct timeval current_time;
                    gettimeofday(&current_time, NULL);
                    
                    long seconds = current_time.tv_sec - start_time.tv_sec;
                    long micros = current_time.tv_usec - start_time.tv_usec;
                    if (micros < 0) {
                        seconds--;
                        micros += 1000000;
                    }
                    long current_ms = seconds * 1000 + micros / 1000;
                    printf("Ok:%d:%ld\n", node_id, elapsed_ms + current_ms);
                } else {
                    printf("Ok:%d:%ld\n", node_id, elapsed_ms);
                }
            }
            else {
                perror("Error: Unknown command for compute node\n");
            }
            

        } else if (strcmp(command, "ping") == 0) {
            printf("Ok: 1 // узел %d доступен\n", node_id);
        } else {
            perror("Error: Unknown command for compute node\n");
        }
        free(command_str);
    }
}

int main() {

    void *zmq_context = zmq_ctx_new();
    control_node_t* control_node = create_control_node();
    printf("Controller Node started. Listening for commands from console...\n");
    char command_buffer[256];

    while (1) {
        if (fgets(command_buffer, sizeof(command_buffer), stdin) == NULL) {
            if (feof(stdin)) {
                printf("Exiting on EOF.\n");
                zmq_ctx_destroy(zmq_context);
                exit(0);
            } else {
                ERROR("Error reading from console.");
                continue;
            }
        }

        command_buffer[strcspn(command_buffer, "\n")] = 0; //Возвращает колличество символов до \n => Просто заменяет \n на \0

        if (strlen(command_buffer) == 0) {
            continue;
        }
        printf("Received command: %s\n", command_buffer);
        char *command = strtok(command_buffer, " "); 

        if (command != NULL) {
            if (strcmp(command, "create") == 0) { // если строки равны

                char *id_str = strtok(NULL, " "); // продолжает поиск в оригинальной строке
                char *parent_id_str = strtok(NULL, " "); 

                if (id_str == NULL || parent_id_str == NULL) {
                    printf("Error: Missing arguments for 'create' command\n");
                    continue;
                }

                int id = atoi(id_str);
                int parent_id = atoi(parent_id_str);

                if (find_node(control_node, id) != NULL) {
                    printf("Error: Already exists\n");
                } else {

                    compute_node_t *new_node = create_compute_node(id, parent_id); 
                    if (parent_id > 0){
                        if (insert_node(find_node(control_node, parent_id), new_node) == NULL){
                            printf("Error: Нет места\n");
                            continue;
                        }
                    }
                    else{
                        insert_node_to_head_node(control_node, new_node);
                    }
                    pid_t pid = fork();
                    if (pid == 0) {
                        compute_node_process(id);
                    } else if (pid > 0) {
                        new_node->pid = pid;
                        char address[50];
                        sprintf(address, "tcp://127.0.0.1:%d", 5550 + id);
                        new_node->socket = zmq_socket(zmq_context, ZMQ_PUSH);
                        int connect_result = zmq_connect(new_node->socket, address);
                        if (connect_result != 0) {
                            ERROR("Failed to connect to compute node, cleanup needed.");
                            printf("Error: Parent is unavailable\n"); 
                        }
                        printf("Ok: %d\n", new_node->pid);
                    } else {
                        ERROR("Fork failed");
                        printf("Error: [System error]\n"); 
                    }
                }
            } else if (strcmp(command, "exec") == 0) {
                char *id_str = strtok(NULL, " ");
                int id = atoi(id_str);
                compute_node_t *node = find_node(control_node, id);
                if (node == NULL) {
                    printf("Error:%d: Not found\n", id);
                } else {
                    char *subcommand = strtok(NULL, "");
                    char exec_command[200];
                    sprintf(exec_command, "exec %s", subcommand);

                    send_command_to_node(node, exec_command);
                }
            } else if (strcmp(command, "ping") == 0) {
                char *id_str = strtok(NULL, " ");

                int id = atoi(id_str);
                compute_node_t *node = find_node(control_node, id);
                if (node == NULL) {
                    printf("Error: Not found\n");
                } else {
                    send_command_to_node(node, "ping");

                }   
            } else {
                printf("Error: [Unknown command]\n");
            }
        }
    }

    return 0;
}

    