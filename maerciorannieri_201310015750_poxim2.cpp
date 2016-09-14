#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

char* print(uint32_t a, uint32_t b);
int main(int argc, char* argv[]) {
	// Ilustrando uso de argumentos de programa
	printf("#ARGS = %i\n", argc);
	printf("PROGRAMA = %s\n", argv[0]);
	printf("ARG1 = %s, ARG2 = %s\n", argv[1], argv[2]);
	// Abrindo arquivos
	FILE* input = fopen(argv[1], "r");
	FILE* output = fopen(argv[2], "w");
	
	int count_line=0;
	char read[10];
	//counting number of lines in the file
	if (input == NULL)
		printf("Arquivo inválido!");
	else{
		while (fscanf(input, "%s\n", read)!=EOF)
			count_line++;
	}
	
	rewind(input); //rebubinando o arquivo - fazendo, novamente,  a leitura do início do arquivo.
		
	//os operandos são utilizados em forma de REGISTROS
	//DEFINE REGISTERS 
	#define PC	R[32]
	#define IR	R[33]
	#define ER	R[34]
	#define FR	R[35]
	#define CR	R[36]
	#define IPC	R[37]

	uint32_t OP; //define the type of operation
	uint32_t R[38] = {0}; //registers >>>  0 - 31 general purpose
//	uint32_t PC = 0; //program counter
//	uint32_t IR; //instruction register
//	uint32_t ER; //extension register 
//	uint32_t FR = 0; //signal register
//	uint32_t CR = 0; //code register
//	uint32_t IPC = 0; //interruption shift register
	uint32_t IM16, IM26; // immediate value 
	uint64_t aux = 0;
	//uint32_t sh = 0;
	bool interrupcao;
	int counter=0; // contador do watchdog
	int *terminal;
    terminal = (int*) malloc (count_line*sizeof(int)); // <<<<<<<<<<<<<VERIFICAR <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//int terminal[5000];
    int count_terminal = 0;
    uint32_t out;   //buffer na situação atual - DO TERMINAL
	
	uint32_t x, y, z, ex, ey, ez; //controladores dos registradores 
	
	//a programação é armazenada em MEMÓRIA
	uint32_t MEMORY[count_line];
	
//	printf("[START OF SIMULATION]\n");	
	fprintf(output, "[START OF SIMULATION]\n");
		
	//reading every line and identifying instructions
	int count=0;
	while (fscanf(input, "%x\n", &MEMORY[count])!=EOF){
		count++;		
	};		
    
    int control=0; //control variable
	
	while (PC < count_line){													
	
	//Watchdog 
        if((FR & 0x00000040) == 0x00000040){ 
            counter--;   
        }
        if(((FR & 0x40) == 0x40) && (counter == 0 )){ // ver como escreve no arquivo em funcao
           	fprintf(output,"[HARDWARE INTERRUPTION 1]\n");        
            PC = 4;
            CR = 0xE1AC04DA;
            fprintf(output,"isr r%01d, r%01d, 0x%04X\n",ex,ey, IM16);        
        	fprintf(output,"[F] R%01d = IPC >> 2 = 0x%08X, R%01d = CR = 0x%08X, PC = 0x%08X\n", ex, R[ex], ey, CR, PC<<2);    
        } 
		IR = MEMORY[PC];
		//applying mask calculation
		OP = (IR & 0xFC000000) >> 26; //mask for operation
		IM16 = (IR & 0x03FFFC00) >> 10; // mask for grouping words
		IM26 = IR & 0x03FFFFFF; // mask for grouping words
		//IM26 = IM26 << 2; utilizado para agrupamento por bytes 
		
		x = (IR & 0x000003E0) >> 5; //mask for register
		y = (IR & 0x0000001F); //mask for register
		z = (IR & 0x00007C00) >> 10; //mask for register
		ex = (IR & 0x00010000) >> 11; //additional mask for type U register
		ex = x + ex;
		ey = (IR & 0x00008000) >> 10;
		ey = y + ey;
		ez = (IR & 0x00020000) >> 12;
		ez = z + ez;
		//o deslocamento é devido a leitura big-endian
								
		switch(OP){				
//			case 0x000000:      //Pseudo-intrução nop                  |    tipo U    |    nenhuma ação é realizada
//			break;			
			case 0x00:      //instrução add  (adição)              |    tipo U    |    R[z] = R[x] + R[y]           |     Campo relacionado: OV (Overflow)								
				if ((R[ez] == 0x00) && (R[ex] == 0x00) && (R[ey] == 0x00)){
					//printf("nop\n");
				}
				else{
					//printf("add\n");
					R[ez] = R[ex] + R[y];
					if ((R[ex] + R[ey]) > 0xFFFFFFFF) {
						FR = FR | 0x00000010;
					}
					else 
						FR = FR & 0xFFFFFFEF;
//						printf("add %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//						printf("[U] FR = 0x%08X, %s = %s + %s = 0x%08X\n", FR, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
						fprintf(output, "add %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));
						fprintf(output, "[U] FR = 0x%08X, %s = %s + %s = 0x%08X\n", FR, print(ez,2), print(ex,2), print(ey,2), R[ez]);
				}																																								
			break;
			
			case 0x01:      //instrução addi (adição imediata)     |    tipo F    |    R[x] = R[y] + IM16           |     Campo relacionado: OV (Overflow)
				//printf("addi\n");
				R[x] = R[y] + IM16;				
				if (R[x] > 0xFFFFFFFF) {
					FR = FR | 0x00000010;
				}
				else 
					FR = FR & 0xFFFFFFEF;
								
//				printf("addi r%01d, r%01d, %01d\n", x, y, IM16);					
//				printf("[F] FR = 0x%08X, R%01d = R%01d + 0x%04X = 0x%08X\n",FR,x,y,IM16,R[x]);	
				fprintf(output, "addi r%01d, r%01d, %01d\n", x, y, IM16);	
				fprintf(output, "[F] FR = 0x%08X, R%01d = R%01d + 0x%04X = 0x%08X\n",FR,x,y,IM16,R[x]);		
			break;
			
			case 0x02:      //instrução sub  (subtração)           |    tipo U    |    R[z] = R[x] - R[y]           |     Campo relacionado: OV (Overflow)
			//use ex;
				//printf("sub\n");
				R[ez] = R[ex] - R[ey];			
		
				if (R[ez] > 0xFFFFFFFF){
//					FR = FR & 0xFFFFFFEF;				
					FR = FR | 0x00000010;										
				}
				else{
//					FR = FR | 0x00000010;					
					FR = FR & 0xFFFFFFEF;		
				}				
							
//				printf("sub %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//				printf("[U] FR = 0x%08X, %s = %s - %s = 0x%08X\n", FR, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
				fprintf(output, "sub %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));		
				fprintf(output, "[U] FR = 0x%08X, %s = %s - %s = 0x%08X\n", FR, print(ez,2), print(ex,2), print(ey,2), R[ez]);					  			    				
			break;
			
			case 0x03:      //instrução subi  (subtração imediata) |    tipo F    |    R[x] = R[y] - IM16           |     Campo relacionado: OV (Overflow)
				//printf("subi\n");
				R[x] = R[y] - IM16;
								
				if (R[x] > 0xFFFFFFFF){
//					FR = FR & 0xFFFFFFEF;				
					FR = FR | 0x00000010;										
				}
				else{
//					FR = FR | 0x00000010;					
					FR = FR & 0xFFFFFFEF;		
				}			
				
//				printf("subi r%01d, r%01d, %d\n", x, y, IM16);
//				printf("[F] FR = 0x%08X, R%01d = R%01d - 0x%04X = 0x%08X\n",FR,x,y,IM16,R[x]);	
				fprintf(output,	"subi r%01d, r%01d, %d\n", x, y, IM16);						
				fprintf(output, "[F] FR = 0x%08X, R%01d = R%01d - 0x%04X = 0x%08X\n",FR,x,y,IM16,R[x]);	
			break;
			
			case 0x04:      //instrução mul  (multiplicação)       |    tipo U    |    R[z] = R[x] * R[y]           |     Campo relacionado: OV (Overflow)
			//use ex;
				//printf("mul\n");			
				R[ez] = R[ex] * R[ey];
				ER = (R[ex] * R[ey]) & 0xFFFFFFFF00000000;						
			
				if (R[ez] > 0xFFFFFFFF){
//					FR = FR & 0xFFFFFFEF;				
					FR = FR | 0x00000010;										
				}
				else{
//					FR = FR | 0x00000010;					
					FR = FR & 0xFFFFFFEF;		
				}			
				
//				printf("mul %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//				printf("[U] FR = 0x%08X, ER = 0x%08X, %s = %s * %s = 0x%08X\n", FR, ER, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
				fprintf(output, "mul %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));		
				fprintf(output, "[U] FR = 0x%08X, ER = 0x%08X, %s = %s * %s = 0x%08X\n", FR, ER, print(ez,2), print(ex,2), print(ey,2), R[ez]);															
			break;
			
			case 0x05:       //instrução muli  (multip imediata)   |    tipo F    |    R[x] = R[y] * IM16           |     Campo relacionado: OV (Overflow)
				//printf("muli\n");	
				aux = (uint64_t)R[y] * (uint64_t)IM16;
				R[x] = (uint32_t)aux;
				ER = (uint32_t)(aux>>32);
				
				if (R[x] > 0xFFFFFFFF){
					FR = FR & 0xFFFFFFEF;				
//					FR = FR | 0x00000010;										
				}
				else{
					FR = FR | 0x00000010;					
//					FR = FR & 0xFFFFFFEF;		
				}			
				
//				printf("muli r%01d, r%01d, %01d\n",x,y,IM16);
//				printf("[F] FR = 0x%08X, ER = 0x%08X, R%01d = R%01d * 0x%04X = 0x%08X\n",FR,ER,x,y,IM16,R[x]);
				fprintf(output,	"muli r%01d, r%01d, %01d\n",x,y,IM16);
				fprintf(output,	"[F] FR = 0x%08X, ER = 0x%08X, R%01d = R%01d * 0x%04X = 0x%08X\n",FR,ER,x,y,IM16,R[x]);											
			break;
			
			case 0x06:       //instrução div  (divisão)            |    tipo U    |    R[z] = R[x] / R[y]   |  Campos relacionados: OV (Overflow) e ZD (divisão por zero)
			//use ex;
				//print("div\n");
				if (R[ey] == 0){
					if ((FR & 0x00000040)==0x00000040){
						CR = 1;
						IPC = PC + 1;
						PC = 2;
					}
					FR = FR & 0xFFFFFFF7;
					FR = FR + (01 << 3);
					ER = 0;					
//					printf("div %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//					printf("[U] FR = 0x%08X, ER = 0x%08X, %s = %s / %s = 0x%08X\n", FR, ER, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
					fprintf(output, "div %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));		
					fprintf(output, "[U] FR = 0x%08X, ER = 0x%08X, %s = %s / %s = 0x%08X\n", FR, ER, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
//					printf("[SOFTWARE INTERRUPTION]\n");
			 		fprintf(output,"[SOFTWARE INTERRUPTION]\n");
				}				
				else{
					FR = 0; 				
					ER = R[ex] % R[ey];
					R[ez] = R[ex] / R[ey];
//					printf("div %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//					printf("[U] FR = 0x%08X, ER = 0x%08X, %s = %s / %s = 0x%08X\n", FR, ER, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
					fprintf(output, "div %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));		
					fprintf(output, "[U] FR = 0x%08X, ER = 0x%08X, %s = %s / %s = 0x%08X\n", FR, ER, print(ez,2), print(ex,2), print(ey,2), R[ez]);												
										
				}																												
			break;
			
			case 0x07:       //instrução divi  (divisão imediata)  |    tipo F    |    R[x] = R[y] / IM16   |  Campos relacionados: OV (Overflow) e ZD (divisão por zero)
				//printf("divi\n");				
				if (IM16 == 0){
					if ((FR & 0x00000040)==0x00000040){
						CR = 1;
						IPC = PC + 1;
						PC = 2;
					}
					FR = FR & 0xFFFFFFF7;
					FR = FR + (01 << 3);
					ER = 0;					
//					printf("[SOFTWARE INTERRUPTION]\n");
			 		fprintf(output,"[SOFTWARE INTERRUPTION]\n");
				}
				else{
					ER = R[y] % IM16;
					R[x] = R[y] / IM16;
				}
//				printf("divi r%01d, r%01d, %01d\n",x,y,IM16);
//				printf("[F] FR = 0x%08X, ER = 0x%08X, R%01d = R%01d / 0x%04X = 0x%08X\n",FR,ER,x,y,IM16,R[x]);		
				fprintf(output, "divi r%01d, r%01d, %01d\n",x,y,IM16);
				fprintf(output,	"[F] FR = 0x%08X, ER = 0x%08X, R%01d = R%01d / 0x%04X = 0x%08X\n",FR,ER,x,y,IM16,R[x]);		
			break;
			
			case 0x08:      //instrução cmp  (comparação)          |    tipo U    |   Campos relacionados: EQ (equal to), LT (less than), GT (great than)
			//use ex;
				//printf("cmp\n");
				if (R[ex] == R[ey]){
						FR = (FR & 0xFFFFFFF8);
						FR = FR + 01;					
				}
				if (R[ex] < R[ey]){
					FR = (FR & 0xFFFFFFF8);
					FR = FR + (01 << 1);				
				}
				if (R[ex] > R[ey]){
					FR = (FR & 0xFFFFFFF8);
					FR = FR + (01 << 2);
				}
								
//				printf("cmp %s, %s\n", print(ex,1), print(ey,1));						
//				printf("[U] FR = 0x%08X\n", FR);												
				fprintf(output, "cmp %s, %s\n", print(ex,1), print(ey,1));
				fprintf(output, "[U] FR = 0x%08X\n", FR);																																
			break;
			
			case 0x09:      //instrução cmpi  (comparação imediata)|    tipo F    |   Campos relacionados: EQ (equal to), LT (less than), GT (great than)
				//printf("cmpi\n");
				if (R[x] == IM16){
					FR = (FR & 0xFFFFFFF8);
					FR = FR + 01;					
				}
				if (R[x] < IM16){
					FR = (FR & 0xFFFFFFF8);
					FR = FR + (01 << 1);
				}
				if (R[x] > IM16){
					FR = (FR & 0xFFFFFFF8);
					FR = FR + (01 << 2);				
				}
//				printf("cmpi r%01d, %01d\n", x,IM16);
//				printf("[F] FR = 0x%08X\n",FR);
				fprintf(output, "cmpi r%01d, %01d\n", x,IM16);
				fprintf(output, "[F] FR = 0x%08X\n",FR);						
			break;
			
			case 0x0A:       //instrução shl  (deslocamento para esquerda)        |    tipo U    			
			//use ex;
			//printf("shl\n");
				aux = ((uint64_t)(ER)<<32) + (uint64_t)(R[ex]);
				aux = aux<<(uint64_t)(ey+1);
				ER = (uint32_t)(aux>>32);
				R[ez] = (uint32_t)aux;

				if (ER != 0){
					FR = (FR & 0xEF);				
					FR = FR + (0x00000010);
				}
				if (ER == 0){
					FR = (FR & 0xEF);					
				}			
						
//				printf("shl %s, %s, %d\n", print(ez,1), print(ex,1), ey);						
//				printf("[U] ER = 0x%08X, %s = %s << %s = 0x%08X\n", ER, print(ez,2), print(ex,2), print(ey+1,2), R[ez]);												
				fprintf(output, "shl %s, %s, %d\n", print(ez,1), print(ex,1), ey);	
				fprintf(output, "[U] ER = 0x%08X, %s = %s << %d = 0x%08X\n", ER, print(ez,2), print(ex,2), ey+1, R[ez]);												
			break;
			
			case 0x0B:       //instrução shr  (deslocamento para direita)         |    tipo U    
			//use ex;
				//printf("shr\n");				
				
				aux = ((uint64_t)(ER)<<32) + (uint64_t)(R[ex]);
				aux = aux>>(uint64_t)(ey+1);
				ER = (uint32_t)(aux>>32);
				R[ez] = (uint32_t)aux;
				
				if (ER != 0){				
					FR = FR + (0x00000010);					
				}
				else{
					FR = FR & 0xFFFFFFEF;
				}

//				printf("shr %s, %s, %d\n", print(ez,1), print(ex,1), ey);						
//				printf("[U] ER = 0x%08X, %s = %s >> %d = 0x%08X\n", ER, print(ez,2), print(ex,2), ey+1, R[ez]);												
				fprintf(output, "shr %s, %s, %d\n", print(ez,1), print(ex,1), ey);
				fprintf(output, "[U] ER = 0x%08X, %s = %s >> %d = 0x%08X\n", ER, print(ez,2), print(ex,2), ey+1, R[ez]);												
			break;
			
			case 0x0C:       //instrução and  (and lógico)         |    tipo U    |    R[z] = R[x] ^ R[y]  
			//use ex;
				//printf("and\n");
				R[ez] = R[ex] & R[ey];
				
//				printf("and %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//				printf("[U] %s = %s & %s = 0x%08X\n", print(ez,2), print(ex,2), print(ey,2), R[ez]);												
				fprintf(output, "and %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
				fprintf(output, "[U] %s = %s & %s = 0x%08X\n", print(ez,2), print(ex,2), print(ey,2), R[ez]);												
														
			break;
			
			case 0x0D:       //instrução andi  (and imediato)      |    tipo F    |    R[x] = R[y] ^ IM16
				//printf("andi\n");
				R[x] = R[y] & IM16;
				
//				printf("andi r%01d, r%01d, %01d\n",x,y,IM16);
//				printf("[F] R%01d = R%01d & 0x%04X = 0x%08X\n",x,y,IM16,R[x]);			
				fprintf(output, "andi r%01d, r%01d, %01d\n",x,y,IM16);
				fprintf(output,	"[F] R%01d = R%01d & 0x%04X = 0x%08X\n",x,y,IM16,R[x]);								
			break;
			
			case 0x0E:       //instrução not  (not lógico)         |    tipo U    |    R[x] = ¬ R[y]
			//use ex;
				//printf("not\n");
				R[ex] = ~R[ey];
				
//				printf("not %s, %s\n", print(ex,1), print(ey,1));						
//				printf("[U] %s = ~%s = 0x%08X\n", print(ex,2), print(ey,2), R[ex]);												
				fprintf(output, "not %s, %s\n", print(ex,1), print(ey,1));						
				fprintf(output, "[U] %s = ~%s = 0x%08X\n", print(ex,2), print(ey,2), R[ex]);												
			break;
			
			case 0x0F:       //instrução noti  (not imediato)      |    tipo F    |    R[x] = ¬ IM16
				//printf("noti\n");
				R[x] = ~IM16;
				
//				printf("noti r%01d, %01d\n",x,IM16);
//				printf("[F] R%01d = ~0x%04X = 0x%08X\n",x,IM16,R[x]);		
				fprintf(output,"noti r%01d, %01d\n",x,IM16);
				fprintf(output,	"[F] R%01d = ~0x%04X = 0x%08X\n",x,IM16,R[x]);
			break;
			
			case 0x10:       //instrução or  (or lógico)           |    tipo U    |    R[z] = R[x] v R[y]
			//use ex;
				//printf("or\n");
				R[ez] = R[ex] | R[ey];
				
//				printf("or %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//				printf("[U] %s = %s | %s = 0x%08X\n", print(ez,2), print(ex,2), print(ey,2), R[ez]);												
				fprintf(output, "or %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
				fprintf(output, "[U] %s = %s | %s = 0x%08X\n", print(ez,2), print(ex,2), print(ey,2), R[ez]);												
			break;
			
			case 0x11:       //instrução ori (or lógico imediato)  |    tipo F    |    R[x] = R[y] v IM16
				//printf("ori\n");
				R[x] = R[y] | IM16;
//				printf("ori r%01d, r%01d, %01d\n",x,y,IM16);
//				printf("[F] R%01d = R%01d | 0x%04X = 0x%08X\n",x,y,IM16,R[x]);		
				fprintf(output, "ori r%01d, r%01d, %01d\n",x,y,IM16);
				fprintf(output,	"[F] R%01d = R%01d | 0x%04X = 0x%08X\n",x,y,IM16,R[x]);						
			break;
			
			case 0x12:       //instrução xor (xor lógico)          |    tipo U    |    
			//use ex;
				//printf("xor\n");
				R[ez] = R[ex] ^ R[ey];
				
//				printf("xor %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
//				printf("[U] %s = %s ^ %s = 0x%08X\n", print(ez,2), print(ex,2), print(ey,2), R[ez]);												
				fprintf(output, "xor %s, %s, %s\n", print(ez,1), print(ex,1), print(ey,1));						
				fprintf(output, "[U] %s = %s ^ %s = 0x%08X\n", print(ez,2), print(ex,2), print(ey,2), R[ez]);												
				
			break;
			
			case 0x13:       //instrução xori (xor imediato)       |    tipo F    |    
				//printf("xori\n");
				R[x] = R[y] ^ IM16;
//				printf("xori r%01d, r%01d, %01d\n",x,y,IM16);
//				printf("[F] R%01d = R%01d ^ 0x%04X = 0x%08X\n",x,y,IM16,R[x]);	
				fprintf(output, "xori r%01d, r%01d, %01d\n",x,y,IM16);
				fprintf(output,	"[F] R%01d = R%01d ^ 0x%04X = 0x%08X\n",x,y,IM16,R[x]);			
			break;
			
			case 0x14:       //instrução ldw (leitura de palavra)  |    tipo F    |    
				//printf("ldw\n");
				R[x] = MEMORY[(R[y] + IM16)];   // VERIFICAR DESLOCAMENTO
				//R[x] = MEMORY[(R[y] + IM16) << 2];
				
				if((R[y] + IM16) == 0x888B){
					R[x] = out;
				}
				
//				printf("ldw r%01d, r%01d, 0x%04X\n",x,y,IM16);
//				printf("[F] R%01d = MEM[(R%01d + 0x%04X) << 2] = 0x%08X\n",x,y,IM16,R[x]);	
				fprintf(output, "ldw r%01d, r%01d, 0x%04X\n",x,y,IM16);
				fprintf(output,	"[F] R%01d = MEM[(R%01d + 0x%04X) << 2] = 0x%08X\n",x,y,IM16,R[x]);									
			break;
			
			case 0x15:       //instrução ldb (leitura de byte)     |    tipo F    |    
				//printf("ldb\n");														
				
				if ((((R[y] + IM16))  % 4)  == 0){				
					R[x] = (MEMORY[ ( (R[y] + IM16) >> 2) ] & 0xFF000000) >> 24;
				
					}
					else
					{						
						if ((((R[y] + IM16))  % 4)  == 1)
							R[x] = (MEMORY[ ( (R[y] + IM16) >> 2) ] & 0x00FF0000) >> 16;							
							else 
							{
								if ((((R[y] + IM16))  % 4)  == 2)
									R[x] = (MEMORY[ ( (R[y] + IM16) >> 2) ] & 0x0000FF00) >> 8;									
									else
									{
										if ((((R[y] + IM16))  % 4)  == 3)
											R[x] = (MEMORY[ ( (R[y] + IM16) >> 2) ] & 0x000000FF);											
									}
							}
					}											
					if((R[y] + IM16) == 0x888B){
						terminal[count_terminal] = R[x];
						count_terminal++;
                    	R[x] = out;
                	}									
//				printf("ldb r%01d, r%01d, 0x%04X\n",x,y,IM16);
//				printf("[F] R%01d = MEM[R%01d + 0x%04X] = 0x%02X\n",x,y,IM16,R[x]);
				fprintf(output, "ldb r%01d, r%01d, 0x%04X\n",x,y,IM16);
				fprintf(output, "[F] R%01d = MEM[R%01d + 0x%04X] = 0x%02X\n",x,y,IM16,R[x]);					
			break;
			
			case 0x16:       //instrução stw (escrita de palavra)  |    tipo F    |    
				//printf("stw\n");	
				counter = (R[y] & 0x3CFFFFFF)+1;
				
				if((R[x] + IM16) == 0x888B){
					terminal[count_terminal] = R[y] & 0x000000FF;
					count_terminal++;
                    out = R[y] & 0x000000FF;
                }				
				
				//MEMORY[(R[x] + IM16)] = R[y];				
//				printf("stw r%01d, 0x%04X, r%01d\n",x,IM16,y);
//				printf("[F] MEM[(R%01d + 0x%04X) << 2] = R%01d = 0x%08X\n",x,IM16,y,R[y]);
				fprintf(output, "stw r%01d, 0x%04X, r%01d\n",x,IM16,y);
				fprintf(output, "[F] MEM[(R%01d + 0x%04X) << 2] = R%01d = 0x%08X\n",x,IM16,y,R[y]);
			break;
			
			case 0x17:       //instrução stb (escrita de byte)     |    tipo F    |  
				//printf("std\n");							
				control = R[y] & 0x000000FF; 
				if(R[y] == 0x888B){
					terminal[count_terminal] = control;
					count_terminal++;
                    out = control;
                }	
				
//				printf("stb r%01d, 0x%04X, r%01d\n",x,IM16,y);
//				printf("[F] MEM[R%01d + 0x%04X] = R%01d = 0x%02X\n",x,IM16,y,control);
				fprintf(output,"stb r%01d, 0x%04X, r%01d\n",x,IM16,y);
				fprintf(output,"[F] MEM[R%01d + 0x%04X] = R%01d = 0x%02X\n",x,IM16,y,control);							
			break;
			
			case 0x18: //push
			//printf("push\n");							
				MEMORY[(R[ex])] = R[ey];
				R[ex] = R[ex] - 1;											
//				printf("push r%01d, r%01d\n", ex, ey);
//				printf("[U] MEM[R%01d--] = R%01d = 0x%08X\n", ex, ey, R[ey]);
				fprintf(output, "push r%01d, r%01d\n", ex, ey);
				fprintf(output, "[U] MEM[R%01d--] = R%01d = 0x%08X\n", ex, ey, R[ey]);
			break;
			
			case 0x19: //pop
			//printf("pop\n");
				R[ey] = R[ey] + 1;								
				R[x] = MEMORY[(R[y])];
//				printf("pop r%01d, r%01d\n", ex, ey);
//				printf("[U] R%01d = MEM[++R%01d] = 0x%08X\n", ex, ey, R[ex]);
				fprintf(output, "pop r%01d, r%01d\n", ex, ey);
				fprintf(output, "[U] R%01d = MEM[++R%01d] = 0x%08X\n", ex, ey, R[ex]);				
			break;
			
			case 0x1A:       //instrução bun (desvio condicional)  |    tipo S    |    		
				//printf("bun\n");	
//				printf("bun 0x%08X\n", IM26);		
				fprintf(output,"bun 0x%08X\n", IM26);	
				PC = IM26 - 1;
				IM26 = IM26 << 2;									
//				printf("[S] PC = 0x%08X\n", IM26);																								
				fprintf(output,"[S] PC = 0x%08X\n", IM26);
				
			break;
				
			case 0x1B:       //instrução beq (desvio condicional)  |    tipo S    |  
				//printf("beq\n");					
//				printf("beq 0x%08X\n",IM26);
				fprintf(output,"beq 0x%08X\n",IM26);
				if (FR & 0x00000001) {
					PC = IM26;
//					printf("[S] PC = 0x%08X\n",PC<<2);				
					fprintf(output,"[S] PC = 0x%08X\n",PC<<2);				
					PC = PC - 1;
				}
				else{
//					printf("[S] PC = 0x%08X\n",(PC+1)<<2);
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);														  
				}					
			break;
			
			case 0x1C:       //instrução blt (desvio condicional)  |    tipo S    |    
				//printf("blt\n");									
//				printf("blt 0x%08X\n",IM26); 
				fprintf(output,"blt 0x%08X\n",IM26); 
				if (FR & 0x00000002){
					PC = IM26;
//					printf("[S] PC = 0x%08X\n",PC<<2);
					fprintf(output,"[S] PC = 0x%08X\n",PC<<2);
					PC = PC - 1;
				}
				else{
//					printf("[S] PC = 0x%08X\n",(PC+1)<<2);
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);
				}								
			break;
			
			case 0x1D:       //instrução bgt (desvio condicional)  |    tipo S    |    
				//printf("bgt\n");								
//				printf("bgt 0x%08X\n",IM26); 							
				fprintf(output,"bgt 0x%08X\n",IM26); 							
				if (FR & 0x00000004){
					PC = IM26;
//					printf("[S] PC = 0x%08X\n",PC<<2);
					fprintf(output,"[S] PC = 0x%08X\n",PC<<2);
					PC = PC - 1;
				}
				else {
//					printf("[S] PC = 0x%08X\n",(PC+1)<<2);
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);
				}									
			break;
			
			case 0x1E:       //instrução bne (desvio condicional)  |    tipo S    |    
				//printf("bne\n");		
//				printf("bne 0x%08X\n",IM26); 
				fprintf(output,"bne 0x%08X\n",IM26); 
				if (FR & 0x00000001) {
//					printf("[S] PC = 0x%08X\n",(PC+1)<<2);
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);
				}
				else{
					PC = IM26;
//					printf("[S] PC = 0x%08X\n",PC<<2);
					fprintf(output,"[S] PC = 0x%08X\n",PC<<2);
					PC = PC - 1;				
				}							
			break;
			
			case 0x1F:       //instrução ble (desvio condicional)  |    tipo S    |    
				//printf("ble\n");
//				printf("ble 0x%08X\n",IM26); 			
				fprintf(output,"ble 0x%08X\n",IM26); 			
				if ( (FR & 0x00000002) | (FR & 0x00000001) ) {
					PC = IM26;
//					printf("[S] PC = 0x%08X\n",PC<<2);
					fprintf(output,"[S] PC = 0x%08X\n",PC<<2);
					PC = PC - 1;
				}
				else{
//					printf("[S] PC = 0x%08X\n",(PC+1)<<2);
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);
				}									
			break;
			
			case 0x20:       //instrução bge (desvio condicional)  |    tipo S    |    
				//printf("bge\n");					
//				printf("bge 0x%08X\n",IM26);			
				fprintf(output,"bge 0x%08X\n",IM26);		
				if (((FR & 0x00000004) == 0x00000004)| ((FR & 0x00000001)==0x00000001)) {
					PC = IM26;
//					printf("[S] PC = 0x%08X\n",PC<<2);
					fprintf(output,"[S] PC = 0x%08X\n",PC<<2);
					PC = PC - 1;
				}
				else{
//					printf("[S] PC = 0x%08X\n",(PC+1)<<2);
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);
				}					
			break;
			
			case 0x21: // bzd - divisão por zero (operação de desvio condicional)
			//printf("bzd\n");
			    //if(((FR &0x00000008)>>3) == 1){
			    if((FR & 0x00000008) == 0x00000008){
        			PC = IM26 - 1;
        			fprintf(output,"bzd 0x%08X\n",IM26);							
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);		        		
    			}
    			else{        		
        			fprintf(output,"bzd 0x%08X\n",IM26);							
					fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);		
    			}					
			break;
			
			case 0x22: //bnz - NÃO divisão por zero (operação de desvio condicional)
			//printf("bnz\n");
				if (!(FR & 0xFFFFFFF7)==0xFFFFFFF7){																	
					PC = IM26;			
				}
//				printf("bnz 0x%08X\n",IM26);
//				printf("[S] PC = 0x%08X\n",PC);		
				fprintf(output,"bnz 0x%08X\n",IM26);
				fprintf(output,"[S] PC = 0x%08X\n",PC);		
			break;
			
			case 0x23: //biv - instrução inválida (operação de desvio condicional) 
			//printf("biv\n");
				if((FR & 0x00000020) == 0x00000020){
					//PC = IM26;
					PC = IM26-1;
				}		
//				printf("biv 0x%08X\n",IM26);
//				printf("biv 0x%08X\n",IM26);			
				fprintf(output,"biv 0x%08X\n",IM26);
				fprintf(output,"[S] PC = 0x%08X\n",(PC+1)<<2);
			break;
			
			case 0x24: //bni - NÃO instrução inválida (operação de desvio condicional) 
			//printf("bni\n");
				if(!(FR & 0x00000020)==0x00000020){
					PC = IM26;
				}
//				printf("bni 0x%08X\n",IM26);
//				printf("[S] PC = 0x%08X\n",PC);																	
				fprintf(output,"bni 0x%08X\n",IM26);
				fprintf(output,"[S] PC = 0x%08X\n",PC);																	
			break;
			
			case 0x25:    //instrução call                          |    tipo F   |    	
			//printf("call\n");
//				printf("call r%01d, r%01d, 0x%04X\n",x,y,IM16);
				fprintf(output,"call r%01d, r%01d, 0x%04X\n",x,y,IM16);							
				R[x] = (PC + 1);
				R[0]=0;
				control = PC;
				PC = (R[y] + IM16);
//				printf("[F] R%01d = (PC + 4) >> 2 = 0x%08X, PC = (R%01d + 0x%04X) << 2 = 0x%08X\n",x,R[x],y,IM16,(PC<<2));
				fprintf(output,"[F] R%01d = (PC + 4) >> 2 = 0x%08X, PC = (R%01d + 0x%04X) << 2 = 0x%08X\n",x,R[x],y,IM16,(PC<<2));				
				PC = PC - 1;
			break;
			
			case 0x26:    //instrução ret                          |    tipo F   |    	
			//printf("ret\n");
				PC = (R[x]);
//				printf("ret r%01d\n",x);
				fprintf(output,"ret r%01d\n",x);
//				printf("[F] PC = R%01d << 2 = 0x%08X\n",x,(PC<<2));
				fprintf(output,"[F] PC = R%01d << 2 = 0x%08X\n",x,(PC<<2));				
				PC = PC - 1;
				
			break;
			
			case 0x27: //isr -  (operação de chamada de rotina de interrupção)
			//printf("isr\n");
//			 	printf("[SOFTWARE INTERRUPTION]\n");
//			 	fprintf(output,"[SOFTWARE INTERRUPTION]\n");
				R[ex] = IPC;
				R[ey] = CR;
				PC = IM16;												
//				printf("isr r%01d, r%01d, 0x%04X\n",ex,ey, IM16);
//				printf("[F] R%01d = IPC >> 2 = 0x%08X, R%01d = CR = 0x%08X, PC = 0x%08X\n", ex, R[ex]>>2, ey, CR, PC<<2);
				fprintf(output,"isr r%01d, r%01d, 0x%04X\n",ex,ey, IM16);
				fprintf(output,"[F] R%01d = IPC >> 2 = 0x%08X, R%01d = CR = 0x%08X, PC = 0x%08X\n", ex, R[ex], ey, CR, PC<<2);				
				PC = PC - 1;
			break;
			
			case 0x3F:       //instrução int (interrupção)         |    tipo S    |    
			//printf("int\n");
//				if (IM26 > 0)
//				{
//					PC = count_line;
//				}								
				if (IM26 == 0){
					fprintf(output,"int %01d\n",IM26); 
					fprintf(output,"[S] CR = 0x%08X, PC = 0x%08X\n",0,IM26);
					PC = count_line;				
					interrupcao = true;
					
					if (count_terminal > 0){
						fprintf(output,"[TERMINAL]\n");
						for (int i=0; i = count_terminal; i++){
							fprintf(output,"%c",terminal[i]);
						}       
						fprintf(output,"[END OF SIMULATION]\n");             
                    }
				}
				
				else{
					IPC = PC + 1;				
					CR = IM26;
					PC = 2;
//					printf("[SOFTWARE INTERRUPTION]\n");
			 		fprintf(output,"int %01d\n",IM26); 
					fprintf(output,"[S] CR = 0x%08X, PC = 0x%08X\n",CR,(PC+1)<<2);
					fprintf(output,"[SOFTWARE INTERRUPTION]\n");
				}			
			break;						
			
			default:
				FR = (FR | 0x00000020);
				//FR = FR + 1;				
				CR = PC;
				IPC = PC + 1;
				fprintf(output,"[INVALID INSTRUCTION @ 0x%08X]\n",(PC<<2));				
				PC = 2;
//				printf("[INVALID INSTRUCTION @ 0x%08X]\n",PC<<2);			
//				fprintf(output,"[INVALID INSTRUCTION @ 0x%08X]\n",(PC<<2));				
//				printf("[SOFTWARE INTERRUPTION]\n");
			 	fprintf(output,"[SOFTWARE INTERRUPTION]\n");
			break;
		}								
		PC++;
	}
	if (interrupcao){ //<<<<<<<<<<<<<<<<<<<<<<<<<< CRIAR INTERRUPÇÃO
//		printf("[END OF SIMULATION]");	
		fprintf(output, "[END OF SIMULATION]");
	} //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<,
	
		
	// Fechando arquivos
	fclose(input);
	fclose(output);
	// Finalizando programa
	return 0;
}

char* print(uint32_t a, uint32_t b){
	char* out = (char*)malloc(2*sizeof(char));
	
	switch(a){
		default:		
			if(b == 1)
				sprintf(out, "r%d", a);
			else 
				sprintf(out, "R%d", a);
			return out;
		break;
		case 0x00000020:	
			if (b == 1)
				strcpy(out, "pc");
			else 
				strcpy(out, "PC");
			return out;
		break;
		case 0x00000021:			
			if (b == 1)
				strcpy(out, "ir");
			else 
				strcpy(out, "IR");
		break;
		case 0x00000022:	
			if (b == 1)
				strcpy(out, "er");
			else 
				strcpy(out, "ER");
		break;
		case 0x00000023:		
			if (b == 1)
				strcpy(out, "fr");
			else 
				strcpy(out, "FR");
		break;			
	}
}
