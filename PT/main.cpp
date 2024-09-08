
/*---------------- File: main.cpp  ---------------------+
|Problema do Transporte (PT)                            |
|					      		                        |
|					      		                        |
| Implementado por: Pedro Garcia, Sávio Francisco       |
+-------------------------------------------------------+ */

#include <bits/stdc++.h>
#include <cstdio>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

//CPLEX Parameters
#define CPLEX_TIME_LIM 3600 //3600 segundos

int ORI; //Quantidade de Origens
int DEST; //Quantidade de destino

vector<int> Origem; //Conjunto de origens
vector<int> Destino;
vector<vector<int>> Custo;

void cplex(){ //CPLEX
    
	IloEnv env; //Define o ambiente do CPLEX

	//Variaveis --------------------------------------------- 
	int numberVar = 0; //Total de Variaveis
	int numberRes = 0; //Total de Restricoes


	//---------- MODELAGEM ---------------

	//Definicao - Variaveis de Decisao 2 dimensoes (x_ij) não binárias (discretas)
	IloArray<IloNumVarArray> x(env);

	for(int i = 0; i < DEST; i++){
		x.add(IloNumVarArray(env));
		for( int j = 0; j < ORI; j++){
			x[i].add(IloIntVar(env, 0, CPXINT_MAX));
			numberVar++;
		}
	}

	//Definicao do ambiente modelo ------------------------------------------
	IloModel model (env);
	IloExpr sum(env); /// Expression for Sum


	//FUNCAO OBJETIVO ---------------------------------------------
	sum.clear();
	for(int i=0; i < DEST; i++ ){
		for (int j=0; j < ORI; j++){
			sum += (Custo[i][j]*x[i][j]);	
		}
	}
	model.add(IloMinimize(env, sum)); //Minimizacao
	
	//RESTRICOES ---------------------------------------------	
	
	//R1 - Atender Demanda
	for(int i = 0; i < DEST; i++){
		sum.clear();
		for(int j = 0; j < ORI; j++){
			sum += x[i][j];
		}
		model.add(sum == Destino[i]); 
		numberRes++;			
	}

	//R2 - Respeitar Oferta
	for(int i = 0; i < ORI; i++){
		sum.clear();
		for(int j = 0; j < DEST; j++){
			sum += x[j][i];	
		}
		model.add(sum <= Origem[i]); 
		numberRes++; 
	}

	//------ EXECUCAO do MODELO ----------
	time_t timer, timer2;
	IloNum value, objValue;
	double runTime;
	string status;
	
	//Informacoes ---------------------------------------------	
	printf("--------Informacoes da Execucao:----------\n\n");
	printf("#Var: %d\n", numberVar);
	printf("#Restricoes: %d\n", numberRes);
	cout << "Memory usage after variable creation:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	
	IloCplex cplex(model);
	cout << "Memory usage after cplex(Model):  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;

	//Setting CPLEX Parameters
	cplex.setParam(IloCplex::TiLim, CPLEX_TIME_LIM);
	//cplex.setParam(IloCplex::TreLim, CPLEX_COMPRESSED_TREE_MEM_LIM);
	//cplex.setParam(IloCplex::WorkMem, CPLEX_WORK_MEM_LIM);
	//cplex.setParam(IloCplex::VarSel, CPLEX_VARSEL_MODE);

	time(&timer);
	cplex.solve();//COMANDO DE EXECUCAO
	time(&timer2);
	
	//cout << "Solution Status: " << cplex.getStatus() << endl;
	//Results
	bool sol = true;
	/*
	Possible Status:
	- Unknown	 
	- Feasible	 
	- Optimal	 
	- Infeasible	 
	- Unbounded	 
	- InfeasibleOrUnbounded	 
	- Error
	*/
	switch(cplex.getStatus()){
		case IloAlgorithm::Optimal: 
			status = "Optimal";
			break;
		case IloAlgorithm::Feasible: 
			status = "Feasible";
			break;
		default: 
			status = "No Solution";
			sol = false;
	}

	cout << endl << endl;
	cout << "Status da FO: " << status << endl;

	if(sol){ 

		//Results
		//int Nbin, Nint, Ncols, Nrows, Nnodes, Nnodes64;
		objValue = cplex.getObjValue();
		runTime = difftime(timer2, timer);
		//Informacoes Adicionais
		//Nbin = cplex.getNbinVars();
		//Nint = cplex.getNintVars();
		//Ncols = cplex.getNcols();
		//Nrows = cplex.getNrows();
		//Nnodes = cplex.getNnodes();
		//Nnodes64 = cplex.getNnodes64();
		//float gap; gap = cplex.getMIPRelativeGap();
		
		cout << "Variaveis de decisao: " << endl;
		
		for(int i = 0; i < DEST; i++){
			for(int j=0; j < ORI; j++){
				value = IloRound(cplex.getValue(x[i][j]));
				printf("x[%d][%d]: %.0lf\n", i+1, j+1, value);
			}
			
		}
		printf("\n");
		
		cout << "Funcao Objetivo Valor = " << objValue << endl;
		printf("..(%.6lf seconds).\n\n", runTime);

	}else{
		printf("No Solution!\n");
	}

	//Free Memory
	cplex.end();
	sum.end();
	

	cout << "Memory usage before end:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	env.end();
}

int main(){

	cin >> ORI >> DEST;
	
	Origem.resize(ORI); //Resize destination vector
	Destino.resize(DEST); //Resize origin vector
	Custo.resize(DEST); //Resize matrix
	
	for(int i=0; i<ORI; i++){
		cin >> Origem[i];
	}
	
	for(int i=0; i<DEST; i++){
		cin >> Destino[i];
	}

	for(int i = 0; i < DEST; i++){
		Custo[i].resize(ORI);
		for(int j = 0; j < ORI; j++){
			cin >> Custo[i][j];
			printf("custo[%d][%d] = %d\n", i, j, Custo[i][j]);
		}

	}

	printf("Verificacao da leitura dos dados:\n");
	printf("Quantidade de Origens: %d\n", ORI);
	printf("Quantidade de Destinos: %d\n", DEST);


	cplex();

    return 0;
}