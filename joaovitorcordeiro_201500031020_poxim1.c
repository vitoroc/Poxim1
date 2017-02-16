#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define BITS 32
uint32_t *memoria;
uint32_t registrador[64];
int num_linhas;

//FILE *arquivo_saida;

#define PC 32
#define IR 33
#define ER 34
#define FR 35

//Leitura da entrada
uint32_t* leitura_do_arquivo(FILE *arq){
	fseek(arq, 0, SEEK_SET);
	int quantidade_de_linhas = 0;
	int linha_escrita = 0;
	uint32_t dado;

	int flag;
	if(!arq){
		printf("Arquivo nao localizado\n");
		return 0;
	}

	do{
		flag = fscanf(arq, "%x", &dado);
		if(flag != EOF){
			quantidade_de_linhas++;
		}
	}while(flag != EOF);

	printf("Quant: %d\n", quantidade_de_linhas);

	fseek(arq, 0, SEEK_SET);
	uint32_t *mem;
	mem = (uint32_t *) malloc(quantidade_de_linhas*sizeof(uint32_t));

	flag = 0;

	do{
		if(flag != EOF){
			flag = fscanf(arq, "%x", &dado);
			mem[linha_escrita] = dado;
			linha_escrita++;
		}
	}while(flag != EOF);

	num_linhas = quantidade_de_linhas;
	return mem;
}

//Retorna codigo da operacao
int returnOP(uint32_t data){
	return data>>26;
}

char retorna1char(int num){
    switch(num){
        case 0: return '0'; break;
        case 1: return '1'; break;
        case 2: return '2'; break;
        case 3: return '3'; break;
        case 4: return '4'; break;
        case 5: return '5'; break;
        case 6: return '6'; break;
        case 7: return '7'; break;
        case 8: return '8'; break;
        default: return '9'; break;
    }
}

char* retornaChar_R(int num){
    int unidade = num % 10;
    int dezena = num/10;
    char* numChar;
    if(dezena == 0){
        numChar = malloc(3*sizeof(char));
        numChar[0] = 'R';
        numChar[1] = retorna1char(unidade);
        numChar[2] = '\0';
        return numChar;
    }
    else{
        numChar = malloc(4*sizeof(char));
        numChar[0] = 'R';
        numChar[1] = retorna1char(dezena);
        numChar[2] = retorna1char(unidade);
        numChar[3] = '\0';
        return numChar;
    }
}

char* retornaChar_r(int num){
    int unidade = num % 10;
    int dezena = num/10;
    char* numChar;
    if(dezena == 0){
        numChar = malloc(3*sizeof(char));
        numChar[0] = 'r';
        numChar[1] = retorna1char(unidade);
        numChar[2] = '\0';
        return numChar;
    }
    else{
        numChar = malloc(4*sizeof(char));
        numChar[0] = 'r';
        numChar[1] = retorna1char(dezena);
        numChar[2] = retorna1char(unidade);
        numChar[3] = '\0';
        return numChar;
    }
}

//Retorna  nome do registrador minusculo
char* return_name_R(int reg){
	if(reg == 32) return (char *)"PC";
	if(reg == 33) return (char *)"IR";
	if(reg == 34) return (char *)"ER";
	if(reg == 35) return (char *)"FR";
	else{
    return retornaChar_R(reg);
	}
}

//Retorna  nome do registrador maiusculo
char* return_name_r(int reg){
	if(reg == 32) return (char *)"pc";
	if(reg == 33) return (char *)"ir";
	if(reg == 34) return (char *)"er";
	if(reg == 35) return (char *)"fr";
	else{
        return retornaChar_r(reg);
	}
}

//Simulador em si
void execute(FILE *arquivo_saida){
	printf("Iniciando simulacao\n");
	fprintf(arquivo_saida, "[START OF SIMULATION]\n");
	printf("[START OF SIMULATION]\n");
	int executa = 1;
	registrador[0] = 0;
	registrador[32] = 0; //PC
	int operation;
	while(executa == 1 && registrador[32] < num_linhas){
		operation = returnOP(memoria[registrador[PC]]); //Acessa a linha armazenada em PC
		registrador[IR] = memoria[registrador[PC]];
		switch(operation){
			case 0:{
				// add
				printf("add\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					uint64_t result = (uint64_t)registrador[Rx] + (uint64_t)registrador[Ry];
					registrador[Rz] = result & 0xFFFFFFFF;
					registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV
					if(result >> 32 == 1) registrador[FR] = registrador[FR] | 0x10; //Caso estoure 32 bits OV recebe 1
				}
				fprintf(arquivo_saida, "add %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] FR = 0x%08X, %s = %s + %s = 0x%08X\n", registrador[FR], return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 1:{
				// addi
				printf("addi\n");
				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					uint64_t result = (uint64_t)registrador[Ry] + IM;
					registrador[Rx] = result & 0xFFFFFFFF;
					registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV
					if(result >> 32 == 1) registrador[FR] = registrador[FR] | 0x10; //Caso estoure 32 bits OV recebe 1
				}
				fprintf(arquivo_saida, "addi %s, %s, %d\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] FR = 0x%08X, %s = %s + 0x%04X = 0x%08X\n", registrador[FR], return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 2:{
				// sub
				printf("sub\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					uint64_t result = (uint64_t)registrador[Rx] - (uint64_t)registrador[Ry];
					registrador[Rz] = result & 0xFFFFFFFF;
					registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV
					if(result >> 32 != 0) registrador[FR] = registrador[FR] | 0x10; //Caso estoure 32 bits OV recebe 1
				}
				fprintf(arquivo_saida, "sub %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] FR = 0x%08X, %s = %s - %s = 0x%08X\n", registrador[FR], return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 3:{
				// subi
				printf("subi\n");
				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				//registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV

				if(Rx != 0){
					uint64_t result = (uint64_t)registrador[Ry] - (uint64_t)IM;
					registrador[Rx] = result & 0xFFFFFFFF;
					registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV
					if(result >> 32 != 0) registrador[FR] = registrador[FR] | 0x10; //Caso estoure 32 bits OV recebe 1
				}
				fprintf(arquivo_saida, "subi %s, %s, %d\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] FR = 0x%08X, %s = %s - 0x%04X = 0x%08X\n", registrador[FR], return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 4:{
				// mul
				printf("mul\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					uint64_t produto = (uint64_t) registrador[Rx] * registrador[Ry];
					registrador[Rz] = produto & 0xFFFFFFFF;
					registrador[ER] = produto >> 32;
					registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV
					if(registrador[ER] != 0) registrador[FR] = registrador[FR] | 0x10;
				}
				fprintf(arquivo_saida, "mul %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] FR = 0x%08X, ER = 0x%08X, %s = %s * %s = 0x%08X\n", registrador[FR], registrador[ER], return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 5:{
				// muli
				printf("muli\n");

				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					uint64_t result = (uint64_t)registrador[Ry] * (uint64_t)IM;
					registrador[Rx] = result & 0xFFFFFFFF;
					registrador[ER] = result >> 32;
					registrador[FR] = registrador[FR] & 0xFFFFFFEF; //Apaga o OV
					if(registrador[ER] != 0) registrador[FR] = registrador[FR] | 0x10;
				}

				fprintf(arquivo_saida, "muli %s, %s, %d\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] FR = 0x%08X, ER = 0x%08X, %s = %s * 0x%04X = 0x%08X\n", registrador[FR], registrador[ER], return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 6:{
				// div
				printf("div\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					if(registrador[Ry] == 0) registrador[FR] = registrador[FR] | 0x8;
					else{
						registrador[ER] = registrador[Rx] % registrador[Ry];
						registrador[Rz] = registrador[Rx] / registrador[Ry];
						registrador[FR] = registrador[FR] & 0xFFFFFFE7; //Apaga o OV e o ZD
					}
				}

				fprintf(arquivo_saida, "div %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] FR = 0x%08X, ER = 0x%08X, %s = %s / %s = 0x%08X\n", registrador[FR], registrador[ER], return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 7:{
				// divi
				printf("divi\n");

				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					if(IM == 0x0) registrador[FR] = registrador[FR] | 0x8; //Seta ZD se reg[Rx] = 0;
					else if(Rx != 0){
						registrador[ER] = registrador[Ry] % IM;
						registrador[Rx] = registrador[Ry] / IM;
						registrador[FR] = registrador[FR] & 0xFFFFFFE7; //Apaga o OV e o ZD
					}
				}

				fprintf(arquivo_saida, "divi %s, %s, %d\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] FR = 0x%08X, ER = 0x%08X, %s = %s / 0x%04X = 0x%08X\n", registrador[FR], registrador[ER], return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 8:{
				// cmp
				printf("cmp\n");

				int Rx, Ry;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				registrador[FR] = registrador[FR] & 0xFFFFFFF8; //Apaga GT, LT, EQ
				if(registrador[Rx] > registrador[Ry]) registrador[FR] = registrador[FR] | 0x4;
				if(registrador[Rx] < registrador[Ry]) registrador[FR] = registrador[FR] | 0x2;
				if(registrador[Rx] == registrador[Ry]) registrador[FR] = registrador[FR] | 0x1;

				fprintf(arquivo_saida, "cmp %s, %s\n", return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] FR = 0x%08X\n", registrador[FR]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 9:{
				//cmpi
				printf("cmpi\n");

				int Rx, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;

				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				registrador[FR] = registrador[FR] & 0xFFFFFFF8; //Apaga GT, LT, EQ
				if(registrador[Rx] > IM) registrador[FR] = registrador[FR] | 0x4;
				if(registrador[Rx] < IM) registrador[FR] = registrador[FR] | 0x2;
				if(registrador[Rx] == IM) registrador[FR] = registrador[FR] | 0x1;

				fprintf(arquivo_saida, "cmpi %s, %d\n", return_name_r(Rx), IM);
				fprintf(arquivo_saida, "[F] FR = 0x%08X\n", registrador[FR]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 10:{
				//shl
				printf("shl\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				uint64_t aux;
				aux = registrador[Rx];
				aux = aux << (Ry + 1);

				registrador[Rz] = aux & 0xFFFFFFFF;
				registrador[ER] = aux >> 32;

				fprintf(arquivo_saida, "shl %s, %s, %d\n", return_name_r(Rz), return_name_r(Rx), Ry);
				fprintf(arquivo_saida, "[U] ER = 0x%08X, %s = %s << %d = 0x%08X\n", registrador[ER], return_name_R(Rz), return_name_R(Rx), Ry+1, registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 11:{
				// shr
				printf("shr\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					uint64_t aux;
					aux = ((uint64_t)registrador[ER] << 32) | registrador[Rx];
					aux = aux >> (Ry + 1);
					registrador[ER] = aux >> 32;
					registrador[Rz] = aux & 0xFFFFFFFF;
				}

				fprintf(arquivo_saida, "shr %s, %s, %d\n", return_name_r(Rz), return_name_r(Rx), Ry);
				fprintf(arquivo_saida, "[U] ER = 0x%08X, %s = %s >> %d = 0x%08X\n", registrador[ER], return_name_R(Rz), return_name_R(Rx), Ry+1, registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 12:{
				//and
				printf("and\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					registrador[Rz] = registrador[Rx] & registrador[Ry];
				}

				fprintf(arquivo_saida, "and %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] %s = %s & %s = 0x%08X\n", return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 13:{
				//andi
				printf("andi\n");

				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					registrador[Rx] = registrador[Ry] & IM;
				}

				fprintf(arquivo_saida, "andi %s, %s, %d\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] %s = %s & 0x%04X = 0x%08X\n",  return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 14:{
				//not
				printf("not\n");

				int Rx, Ry;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				if(Rx != 0){
					registrador[Rx] = ~registrador[Ry];
				}

				fprintf(arquivo_saida, "not %s, %s\n", return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] %s = ~%s = 0x%08X\n", return_name_R(Rx), return_name_R(Ry), registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 15:{
				//noti
				printf("noti\n");

				int Rx, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					registrador[Rx] = ~IM;
				}

				fprintf(arquivo_saida, "noti %s, %d\n", return_name_r(Rx), IM);
				fprintf(arquivo_saida, "[F] %s = ~0x%04X = 0x%08X\n",  return_name_R(Rx), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 16:{
				//or
				printf("or\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					registrador[Rz] = registrador[Rx] | registrador[Ry];
				}

				fprintf(arquivo_saida, "or %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] %s = %s | %s = 0x%08X\n", return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 17:{
				//ori
				printf("ori\n");

				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					registrador[Rx] = registrador[Ry] | IM;
				}

				fprintf(arquivo_saida, "ori %s, %s, %u\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] %s = %s | 0x%04X = 0x%08X\n", return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 18:{
				//xor
				printf("xor\n");

				int Rx, Ry, Rz;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				Rz = memoria[registrador[PC]] >> 10 & 0x1F;
				Rz = Rz | ((memoria[registrador[PC]] & 0x20000)>>12);

				if(Rz != 0){
					registrador[Rz] = registrador[Rx] ^ registrador[Ry];
				}

				fprintf(arquivo_saida, "xor %s, %s, %s\n", return_name_r(Rz), return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] %s = %s ^ %s = 0x%08X\n", return_name_R(Rz), return_name_R(Rx), return_name_R(Ry), registrador[Rz]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 19:{
				//xori
				printf("xori\n");

				int Rx, Ry, IM;

				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					registrador[Rx] = registrador[Ry] ^ IM;
				}

				fprintf(arquivo_saida, "xori %s, %s, %u\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] %s = %s ^ 0x%04X = 0x%08X\n", return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 20:{
				// ldw
				printf("ldw\n");

				int Rx, Ry, IM;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					registrador[Rx] = memoria[registrador[Ry] + IM];
				}

				fprintf(arquivo_saida, "ldw %s, %s, 0x%04X\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] %s = MEM[(%s + 0x%04X) << 2] = 0x%08X\n",  return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 21:{
				//ldb

				printf("ldb\n");

				int Rx, Ry, IM;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					//Aplica a mascara correta
					switch((registrador[Ry] + IM) % 4){
						case 0:{registrador[Rx] = (memoria[(registrador[Ry] + IM) / 4] & 0xFF000000) >> 24;break;}
						case 1:{registrador[Rx] = (memoria[(registrador[Ry] + IM) / 4] & 0xFF0000) >> 16;  break;}
						case 2:{registrador[Rx] = (memoria[(registrador[Ry] + IM) / 4] & 0xFF00) >> 8;	  break;}
						case 3:{registrador[Rx] = memoria[(registrador[Ry] + IM) / 4] & 0xFF;	  break;}
					}
				}

				fprintf(arquivo_saida, "ldb %s, %s, 0x%04X\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] %s = MEM[%s + 0x%04X] = 0x%02X\n",  return_name_R(Rx), return_name_R(Ry), IM, registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 22:{
				// stw
				printf("stw\n");

				int Rx, Ry, IM;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				memoria[registrador[Rx]+IM] = registrador[Ry];

				fprintf(arquivo_saida, "stw %s, 0x%04X, %s\n", return_name_r(Rx), IM, return_name_r(Ry));
				fprintf(arquivo_saida, "[F] MEM[(%s + 0x%04X) << 2] = %s = 0x%08X\n", return_name_R(Rx), IM, return_name_R(Ry), registrador[Ry]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 23:{
				//stb
				printf("stb\n");

				int Rx, Ry, IM;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				switch((registrador[Rx] + IM) % 4){
					case 0:{memoria[(registrador[Rx] + IM) / 4] = (registrador[Ry] | 0xFF000000) >> 24;break;}
					case 1:{memoria[(registrador[Rx] + IM) / 4] = (registrador[Ry] | 0xFF0000) >> 16;  break;}
					case 2:{memoria[(registrador[Rx] + IM) / 4] = (registrador[Ry] | 0xFF00) >> 8;	  break;}
					case 3:{memoria[(registrador[Rx] + IM) / 4] = registrador[Ry] | 0xFF;	  break;}
				}

				fprintf(arquivo_saida, "stb %s, 0x%04X, %s\n", return_name_r(Rx), IM, return_name_r(Ry));
				fprintf(arquivo_saida, "[F] MEM[%s + 0x%04X] = %s = 0x%02X\n", return_name_R(Rx), IM, return_name_R(Ry), registrador[Ry]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 24:{
				//push
				printf("push\n");
				//Implementar OV
				
				int Rx, Ry;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				memoria[registrador[Rx]] = registrador[Ry]; //stw
				if(Rx != 0){								//subi
					registrador[Rx]--;
				}


				fprintf(arquivo_saida, "push %s, %s\n", return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] MEM[%s--] = %s = 0x%08X\n", return_name_R(Rx), return_name_R(Ry), registrador[Ry]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 25:{
				//pop
				printf("pop\n");

				int Rx, Ry;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Rx = Rx | ((memoria[registrador[PC]] & 0x10000)>>11);

				Ry = memoria[registrador[PC]] & 0x1F;
				Ry = Ry | ((memoria[registrador[PC]] & 0x8000)>>10);

				if(Rx != 0){									//addi
					registrador[Ry]++;
					registrador[Rx] = memoria[registrador[Ry]]; //ldw
				}

				fprintf(arquivo_saida, "pop %s, %s\n", return_name_r(Rx), return_name_r(Ry));
				fprintf(arquivo_saida, "[U] %s = MEM[++%s] = 0x%08X\n", return_name_R(Rx), return_name_R(Ry), registrador[Rx]);

				registrador[PC]++; //PC recebe proxima linha
				break;
			}

			case 26:{
				// bun
				printf("bun\n");
				uint32_t IM;
				IM = memoria[registrador[PC]] & 0x3FFFFFF;

				registrador[PC] = IM;

				fprintf(arquivo_saida, "bun 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", IM << 2);

				break;
			}

			case 27:{
				//beq
				printf("beq\n");

				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;

				if(registrador[FR] & 0x1)	registrador[PC] = IM;
				else	registrador[PC]++;

				fprintf(arquivo_saida, "beq 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", registrador[PC] << 2);

				break;
			}

			case 28:{
				//blt
				printf("blt\n");

				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;

				if(registrador[FR] & 0x2)	registrador[PC] = IM;
				else	registrador[PC]++;

				fprintf(arquivo_saida, "blt 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", registrador[PC] << 2);

				break;
			}

			case 29:{
				// bgt
				printf("bgt\n");

				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;

				if(registrador[FR] & 0x4)	registrador[PC] = IM;
				else	registrador[PC]++;

				fprintf(arquivo_saida, "bgt 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", registrador[PC] << 2);

				break;
			}

			case 30:{
				//bne
				printf("bne\n");

				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;
				if(!(registrador[FR] & 0x1))	registrador[PC] = IM;
				else	registrador[PC] = registrador[PC] + 1;

				fprintf(arquivo_saida, "bne 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", registrador[PC] << 2);

				break;
			}

			case 31:{
				//ble
				printf("ble\n");

				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;
				if((registrador[FR] & 0x1) | (registrador[FR] & 0x2))	registrador[PC] = IM;
				else	registrador[PC] = registrador[PC] + 1;

				fprintf(arquivo_saida, "ble 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", registrador[PC] << 2);

				break;
			}

			case 32:{
				//bge
				printf("bge\n");

				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;
				if((registrador[FR] & 0x1) | (registrador[FR] & 0x4))	registrador[PC] = IM;
				else	registrador[PC] = registrador[PC] + 1;

				fprintf(arquivo_saida, "bge 0x%08X\n", IM);
				fprintf(arquivo_saida, "[S] PC = 0x%08X\n", registrador[PC] << 2);

				break;
			}

			case 37:{
				//call
				printf("call\n");

				int Rx, Ry, IM;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;
				Ry = memoria[registrador[PC]] & 0x1F;
				IM = memoria[registrador[PC]] >> 10 & 0xFFFF;

				if(Rx != 0){
					registrador[Rx] = registrador[PC] + 1;
				}
				registrador[PC] = registrador[Ry] + IM;

				fprintf(arquivo_saida, "call %s, %s, 0x%04X\n", return_name_r(Rx), return_name_r(Ry), IM);
				fprintf(arquivo_saida, "[F] %s = (PC + 4) >> 2 = 0x%08X, PC = (%s + 0x%04X) << 2 = 0x%08X\n", return_name_R(Rx), registrador[Rx], return_name_R(Ry), IM, registrador[PC]<<2);

				break;
			}

			case 38:{
				//ret
				printf("ret\n");

				int Rx;
				Rx = memoria[registrador[PC]] >> 5 & 0x1F;

				registrador[PC] = registrador[Rx];

				fprintf(arquivo_saida, "ret %s\n", return_name_r(Rx));
				fprintf(arquivo_saida, "[F] PC = %s << 2 = 0x%08X\n", return_name_R(Rx), registrador[PC] << 2);

				break;
			}

			case 63:{

				//EDITAR AQUIIIIIIIIII


				// int
				printf("int\n");
				uint32_t IM = memoria[registrador[PC]] & 0x3FFFFFF;
				if(IM == 0){
					executa = 0;
					registrador[PC] = 0;
				}
				else registrador[PC] = registrador[PC] + 1;

				fprintf(arquivo_saida, "int %u\n", IM);
				fprintf(arquivo_saida, "[S] CR = 0x%08X, PC = 0x%08X\n", 0, registrador[PC]);

				break;
			}

			default:{
				printf("DEFALT\n");
				executa = 0;
				break;
			}
		}
	}
	fprintf(arquivo_saida, "[END OF SIMULATION]\n");
	printf("[END OF SIMULATION]\n");
}


int main(int argc, char *argv[]){
	printf("Iniciado\n");

	FILE *arquivo;
	FILE *arquivo_saida;

	registrador[0] = 0;
	registrador[PC] = 0;
	registrador[FR] = 0x0;

	arquivo = fopen(argv[1], "r");

	printf("Saida: %s\n", argv[2]);

	arquivo_saida = fopen(argv[2], "w");

	if(!arquivo_saida){
		printf("Arquivo de saida nao inicializado\n");
		return 0;
	}

	memoria = leitura_do_arquivo(arquivo);
	printf("Memoria carregada\n");

	printf("Entrada:\n");
	int i;
	for(i = 0; i < num_linhas; i++)
		printf("0x%08X\n", memoria[i]);

	execute(arquivo_saida);

	printf("Liberando ponteiros\n");
	free(memoria);
	memoria = NULL;
	fclose(arquivo);
	fclose(arquivo_saida);
	printf("Fechando arquivos\n");
	printf("Programa encerrado\n");
	return 0;
}
