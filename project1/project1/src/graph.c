#include "graph.h"

struct AdjListNode* newAdjListNode(int dest)
{ 
    struct AdjListNode* newNode = (struct AdjListNode*) malloc(sizeof(struct AdjListNode)); 
    newNode->dest = dest; 
    newNode->next = NULL; 
    return newNode; 
} 

struct DepGraph* createDepGraph(FILE* input, char cmds[][550])
{
    ssize_t read;
    size_t len = 0;
    char* line = NULL;
    int V;

    // Gets the number of nodes
    read = getline(&line, &len, input);
    sscanf(line, "%d", &V);

        // Error checking for number of nodes/commands
    
        if (V < 1 || V > 32) {

            printf("ERROR: An invalid number of nodes were entered...\n");
            exit(1);
        }

        // skip the first blank line
        read = getline(&line, &len, input);

        int cmdIter = 0;
        int j;

        // Aquire Command Lines
        for (j = 0; j < V; j++)
        {
            read = getline(&line, &len, input);

            //Error checking for command length
            if (len > 552) // Maximum allowed characters in input command + NULL character '\0'
            {
                printf("ERROR: Exceeded maximum number characters for input command string...\n");
                exit(1);
            }
            strcpy(cmds[cmdIter++], line);
        }

        // Skip second blank line
        read = getline(&line, &len, input);

        // Graph creation
        struct DepGraph* graph = (struct DepGraph*)malloc(sizeof(struct DepGraph));
        graph->V = V;
        graph->array = (struct AdjList*)malloc(V * sizeof(struct AdjList));

        int i;

        // Initialize Each Node Structure
        for (i = 0; i < V; ++i)
        {
            graph->array[i].head = NULL;
            graph->array[i].visit = 0;

        }

        char* holder;
        int src;
        int dest;


        // Aquire SRC and DEST values
        while ((read = getline(&line, &len, input)) != -1)
        {
            holder = strtok(line, " ");

            src = atoi(holder);

            holder = strtok(NULL, " ");

            dest = atoi(holder);

            // Error checking for dest and src values (it is assumed that src > dest)
            if (dest < src)
            {
                printf("ERROR: Destination Node cannot be less than Source Node...\n");
                exit(1);
            }

            // Error checking for dest and src values (it is assumed that these values can not be negative)
            if (src < 0 || dest < 0)
            {
                printf("ERROR: Both Source and Destination Nodes must be greater than or equal to 0...\n");
                exit(1);
            }

            // addEdge function that prevents infinite recursion
            if (dest == src)
            {
                addEdge(graph, src, -1);
            }

            // Standard call for addEdge, src and dest are non-negative and dest > src
            if (dest > src && src >= 0)
            {
                addEdge(graph, src, dest);
            }
        }

        return graph;
    
}


void addEdge(struct DepGraph* graph, int src, int dest)
{

    struct AdjListNode* newNode;
    
    // Prevention of infinite recursion, node will not recieve a child
    if (dest == -1)
    {
        newNode = NULL;
    }
    else
    {
        newNode = newAdjListNode(dest);
    }
    struct AdjListNode* current = graph->array[src].head;
    
    // Inserts newNode into graph
    if (graph->array[src].head == NULL)
    {
        graph->array[src].head = newNode;

        current = graph->array[src].head;
    } else 
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newNode;
    }
	
}

void DFSVisit(struct DepGraph* graph, int node, char cmds[][550], int mode)
{
    // Start from node, iterate over its adjListNode
    struct AdjListNode* adjListNode = graph -> array[node].head;
    
    int PID;

    while(adjListNode != NULL)
    {

        PID = fork();

        if (PID < 0) // error
        {
            printf("ERROR: Forking error...");
            exit(1);
        }
        else if (PID == 0) // child process
        {

            DFSVisit(graph, adjListNode->dest, cmds, mode); 

        }
        else // parent process
        {
            if (!mode) // serial
            {
                wait(NULL);
            }
        }
        adjListNode = adjListNode->next;
    }

    if(mode){ while (wait(NULL) > 0); } /*wait for all children before continuation*/

    FILE* output;

    output = fopen("results.txt", "a");
    
    //Error checking for file error
    if (!output)
    {
        printf("File %s not able to be written...\n", "results.txt");
    }
    if(!output) {
		
        printf("ERROR: File %s not able to be written...\n", "results.txt");
		printf("Exiting...\n");
		exit(1);

	}

    pid_t personalPID = getpid();
    pid_t parentPID = getppid();
    
    //Error checking for pid functions
    if (personalPID < -1)
    {
        printf("ERROR: Unable to aquire PID of child...\n");
        exit(1);
    }
    if (parentPID < -1)
    {
        printf("ERROR: Unable to aquire parent PID of child...\n");
        exit(1);
    }

    fprintf(output, "%d %d %s", personalPID, parentPID, cmds[node]);

    fclose(output);
    
    char * commandSplit[50];
    int counter = 0;
    
    // Gets the first argument of the command (the command itself)
    commandSplit[0] = strtok(cmds[node], " \n\r");

    // Get the rest of the arguments
    while (commandSplit[counter] != NULL)
    {

        counter = counter + 1;
        commandSplit[counter] = strtok(NULL, " \n\r");
        
    }

    commandSplit[counter] = NULL; //terminating null

    execvp(commandSplit[0],commandSplit);
    
    printf("ERROR: Execution failed, exiting...\n");

    exit(0);
}

void processGraph(struct DepGraph* graph, char cmds[][550], int mode)
{
    int i;
    int child = fork();
    
    if (child < 0) //error
    { 
        printf("ERROR: Forking error...");
        exit(1);
    }
    if (child == 0) //child
    { 
	   DFSVisit(graph, 0, cmds, mode);
    }
    else //parent
    { 
        wait(NULL);
    }
}

// THIS IS A TEST FUNCTION I MADE TO CONFIRM THE ADJLIST IS FUNCTIONAL
void printAdjList(struct DepGraph* graph, char cmds[][550], int mode)
{
    struct AdjListNode *head = NULL;
    int i;
    for (i = 0; i < graph->V; i++) {

        head = graph->array[i].head;

        if (head == NULL) {

        }
        else {

            printf("AdjList of Node %d : %d", i,head->dest);

            while (head->next != NULL) {

                
                head = head->next;
                printf("->%d", head->dest);

            }

            printf("\n");

        }
    }

}
