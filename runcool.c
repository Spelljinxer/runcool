//  CITS2002 Project 1 2021
//  Name(s):             Reiden Rufin, Nathan Eden
//  Student number(s):   22986337, 22960674

//  compile with:  cc -std=c11 -Wall -Werror -o runcool runcool.c

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//  THE STACK-BASED MACHINE HAS 2^16 (= 65,536) WORDS OF MAIN MEMORY
#define N_MAIN_MEMORY_WORDS (1<<16)

//  EACH WORD OF MEMORY CAN STORE A 16-bit UNSIGNED ADDRESS (0 to 65535)
#define AWORD               uint16_t
//  OR STORE A 16-bit SIGNED INTEGER (-32,768 to 32,767)
#define IWORD               int16_t

//  THE ARRAY OF 65,536 WORDS OF MAIN MEMORY
AWORD                       main_memory[N_MAIN_MEMORY_WORDS];

//  THE SMALL-BUT-FAST CACHE HAS 32 WORDS OF MEMORY
#define N_CACHE_WORDS       32


//  see:  https://teaching.csse.uwa.edu.au/units/CITS2002/projects/coolinstructions.php
enum INSTRUCTION {
    I_HALT       = 0,
    I_NOP,
    I_ADD,
    I_SUB,
    I_MULT,
    I_DIV,
    I_CALL,
    I_RETURN,
    I_JMP,
    I_JEQ,
    I_PRINTI,
    I_PRINTS,
    I_PUSHC,
    I_PUSHA,
    I_PUSHR,
    I_POPA,
    I_POPR
};

//  USE VALUES OF enum INSTRUCTION TO INDEX THE INSTRUCTION_name[] ARRAY
const char *INSTRUCTION_name[] = {
    "halt",
    "nop",
    "add",
    "sub",
    "mult",
    "div",
    "call",
    "return",
    "jmp",
    "jeq",
    "printi",
    "prints",
    "pushc",
    "pusha",
    "pushr",
    "popa",
    "popr"
};

//  ----  IT IS SAFE TO MODIFY ANYTHING BELOW THIS LINE  --------------


//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads     = 0;
int n_main_memory_writes    = 0;
int n_cache_memory_hits     = 0;
int n_cache_memory_misses   = 0;



void report_statistics(void)
{
    printf("@fast-jeq-implemented\n");
    printf("@number-of-main-memory-reads\t%i\n",    n_main_memory_reads);
    printf("@number-of-main-memory-writes\t%i\n",   n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t%i\n",    n_cache_memory_hits);
    printf("@number-of-cache-memory-misses\t%i\n",  n_cache_memory_misses);
}


//  -------------------------------------------------------------------

//use functions read_memory() and write_memory() instead of accessing main_memory[] directly

//  Initialise arrays for cache implementation:
//      - dirty = checks if the value inside cache is dirty
//      - addresses = addresses inside the cache (these hold the values)
//      - cache = the value inside each cache

bool dirty[N_CACHE_WORDS];
int addresses[N_CACHE_WORDS];
IWORD cache [N_CACHE_WORDS];

//function to initalise the values inside the cache as all dirty
// also sets the addresses inside the cache to the maximum main_memory words
// call within main()
void fillDirtyAndAddresses()
{
    for(int i = 0; i < N_CACHE_WORDS; i++)
    {
        dirty[i] = true;
        addresses[i] = N_MAIN_MEMORY_WORDS;
    }
}


//Takes the INPUT - ADDRESS and uses it to read the value
AWORD read_memory(int address)
{
    int cacheIndex = address % 32; // the index of the cache is address % 32
    
    if(addresses[cacheIndex] == address) 
    {
        ++n_cache_memory_hits;  //If the address is inside the cache, we get a cache hit 
        return cache[cacheIndex];
    }
    else
    {
        if(dirty[cacheIndex] == true) 
        {
            int addressVal = addresses[cacheIndex];
            main_memory[addressVal] = cache[cacheIndex];
            ++n_main_memory_writes;                     
            dirty[cacheIndex] = false;                  //set their values to clean 
        }
        cache[cacheIndex] = main_memory[address];   //We then get a cache miss and store the value and address from main_memory
        addresses[cacheIndex] = address;            
        ++n_main_memory_reads;
        ++n_cache_memory_misses;  //Cache miss = increment both reads and misses, as we do not read from cache but from main_memory
        
    }
    
    return cache[cacheIndex];
}

//Uses the INPUT - VALUE to write into the INPUT - ADDRESS
void write_memory(AWORD address, AWORD value)
{
    int cacheIndex = address % 32; // the index of the cache is address % 32
    
    if(addresses[cacheIndex] == address) //Checks if current address is found within the cache, set the cache to the INPUT - VALUE
    {
        cache[cacheIndex] = value; //Stores the value inside the cache
        dirty[cacheIndex] = true;  //Set it to now dirty
    }
    else if(dirty[cacheIndex] == true) //similarly to write_memory, we clean the data
    {
        int addressVal = addresses[cacheIndex];
        main_memory[addressVal] = cache[cacheIndex];  
        ++n_main_memory_writes; 
        addresses[cacheIndex] = address;
        cache[cacheIndex] = value;
        dirty[cacheIndex] = true;   //set them as dirty
    }
    else
    {
        cache[cacheIndex] = value; //Stores the value and address inside the cache
        addresses[cacheIndex] = address; 
        dirty[cacheIndex] = true; //Set it to now dirty
    }
}



//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                    // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // frame pointer
    
    while(true) 
    {
        //use AWORD as addresses cannot go into negatives
        AWORD addressValue1;
        AWORD addressValue2; 
        
        //use IWORD as these values can go into negatives
        IWORD value1;
        IWORD value2; 
        
        //  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction   = read_memory(PC);
        ++PC;

        if(instruction == I_HALT)
        {
            break;
        }
        //Pushing onto stack = --SP;
        //Popping from stack = ++SP;
        switch(instruction)
        {   
            case I_NOP:
                break;

            case I_ADD:
                value1 = read_memory(SP);
                ++SP;
                value2 = read_memory(SP);
                write_memory(SP, value1 + value2);
                
                break;

            case I_SUB:
                value1 = read_memory(SP);
                ++SP;
                value2 = read_memory(SP);

                write_memory(SP, value2 - value1); //value 2 is at the TOS, so v2 - v1 instead of v1 - v2
                break;

            case I_MULT:
                value1 = read_memory(SP);
                ++SP;
                value2 = read_memory(SP);
                
                write_memory(SP, value1 * value2);
                break;

            case I_DIV:
                value1 = read_memory(SP);
                ++SP;
                value2 = read_memory(SP);
                
                write_memory(SP, value2 / value1); //value 2 is at the TOS, so v2 / v1 instead of v1 / v2
                break;

            case I_CALL:
                addressValue1 = read_memory(PC); 
                ++PC;
                --SP;
                write_memory(SP, PC); //return address saved onto stack 
                
                PC = addressValue1; 
                --SP;
                write_memory(SP, FP); //current value of FP pushed onto stack 
                FP = SP; //copy current value of SP to FP 
                break;

            case I_RETURN:
                
                addressValue1 = read_memory(PC) + FP; //offset
                
                value1 = read_memory(SP); //return  value
                PC = read_memory(FP + 1); //possibility of function having no parameter - 
                                          // copy to the same stack location as the return address
                write_memory(addressValue1, value1);

                FP = read_memory(FP); //FP becomes the saved previous FP - caller's stack frame
                SP = addressValue1;
                break;

            case I_JMP:
                addressValue1 = read_memory(PC);
                PC = addressValue1; //Jump to the address of the next word
                break;

            case I_JEQ:
                value1 = read_memory(SP);
                ++SP;
                addressValue1 = read_memory(PC);
               
                if (value1 == 0)
                {
                    PC = addressValue1; //Flow of execution continues at the address following 'jeq'
                }
                else
                {
                    ++PC;
                }

                break;

            case I_PRINTI:
                value1 = read_memory(SP);
                ++SP;
                printf("%d", value1); //prints to stdout
                break;

            case I_PRINTS:
                addressValue1 = read_memory(PC);
                
                value1 = read_memory(addressValue1);

                int firstChar = (value1 >> 0); 
                int secondChar = (value1 >> 8);    //bitshifting and split both of the addresses, checking each one

                printf("%c%c", firstChar,secondChar); //prints to stdout

                while(firstChar != 0 && secondChar != 0)   //terminate once we reach the nullbyte character '\0'
                {
                    addressValue1++;
                    value1 = read_memory(addressValue1);   //keep checking the next char by incrementing addressValue1
            
                    firstChar = (value1 >> 0);
                    secondChar = (value1 >> 8);

                    printf("%c%c", firstChar,secondChar); //prints to stdout
                    
                }
                ++PC;
                break;

            case I_PUSHC:
                value1 = read_memory(PC);
                ++PC;
                --SP;
                write_memory(SP, value1);
                break;

            case I_PUSHA:
                value1 = read_memory(PC); //address of the integer to be pushed onto stack 
                ++PC;
                --SP;
                value2 = read_memory(value1); //retrieves the integer
                write_memory(SP, value2);
                break;

            case I_PUSHR:
                value1 = read_memory(PC); //offset
                ++PC;
                --SP;
                value2 = read_memory(FP + value1); //plain address
                write_memory(SP, value2);
                break;

             case I_POPA:
                addressValue2 = read_memory(SP); 
                ++SP; 
                addressValue1 = read_memory(PC+1); //address into which the value is popped
                ++PC;
                write_memory(addressValue2, addressValue1); 
                break;

            case I_POPR:
                value1 = read_memory(PC); //offset
                ++PC;
                addressValue2 = (FP + value1); //plain address
                value2 = read_memory(SP);
                ++SP;
                write_memory(addressValue2, value2);
                break;
        }
    }
//  THE RESULT OF EXECUTING THE INSTRUCTIONS IS FOUND ON THE TOP-OF-STACK
    return read_memory(SP);
}

//  -------------------------------------------------------------------

//  READ THE PROVIDED coolexe FILE INTO main_memory[]
void read_coolexe_file(char filename[])
{
    memset(main_memory, 0, sizeof main_memory);   //  clear all memory
    
    FILE *fp = fopen(filename, "rb+"); //"rb+" is used as coolexe is in binary
    fread(main_memory, 2, N_MAIN_MEMORY_WORDS, fp);
    
    fclose(fp);
}

//  -------------------------------------------------------------------

int main(int argc, char *argv[])
{   
    
//  CHECK THE NUMBER OF ARGUMENTS
    if(argc != 2) {
        fprintf(stderr, "Usage: %s program.coolexe\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //execute the function to fill cache and address values
    fillDirtyAndAddresses();
//  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);
    
//  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics();
    printf("exit(%i)\n", result);
    return result;          
}
