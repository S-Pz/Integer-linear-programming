
#include <bits/stdc++.h>
#include <cstdio>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

//CPLEX Parameters
#define CPLEX_TIME_LIM 3600 //3600 segundos

int n_linha; //Quantidade de vertices
int n_colun;

vector<vector<int>> Grafo;

void cplex(){ //CPLEX
    
	IloEnv env; //Define o ambiente do CPLEX

	//Variaveis --------------------------------------------- 
	int numberVar = 0; //Total de Variaveis
	int numberRes = 0; //Total de Restricoes


	//---------- MODELAGEM ---------------

	//Definicao - Variaveis de Decisao 2 dimensoes (x_ij) não binárias (discretas)
	IloArray<IloNumVarArray> x(env);

	for(int i = 0; i < n_linha; i++){
		x.add(IloNumVarArray(env));
		for( int j = 0; j < n_colun; j++){
			x[i].add(IloIntVar(env, 0, 1));
			numberVar++;
		}
	}

	//Definicao do ambiente modelo ------------------------------------------
	IloModel model (env);
	IloExpr sum(env); /// Expression for Sum
	IloExpr sum2(env);

	//FUNCAO OBJETIVO ---------------------------------------------
	sum.clear();
	for(int s=0; s < n_linha; s++ ){
		for (int j=0; j < n_colun; j++){
			if(s==j || Grafo[s][j] == 0){
				continue;
			}
			printf("custo[%c][%c] = %d\n", 65+s, 65+j, Grafo[s][j]);
			sum += Grafo[s][j]*x[s][j];	
		}
	}
	model.add(IloMaximize(env, sum)); //Minimizacao
	
	//RESTRICOES ---------------------------------------------	
	
	//R1 Conservação de fluxo
	for(int i = 0; i < n_linha; i++){
		
		if(i!=0 && i!= n_linha-1){

			sum.clear();
			for(int j = 0; j < n_colun; j++){
				if(i==j) continue;

				if(Grafo[i][j]!=0){
					sum += x[i][j];
				}
			}
			sum2.clear();
			for (int k = 0; k < n_colun; k++){
				if(i==k) continue;

				if(Grafo[k][i] != 0){
					sum2 += x[k][i];
				}
				// printf("custo[%c][%c] = %d\n", 65+k, 65+i, Grafo[k][i]);
			}
			model.add(sum - sum2 == 0); 
			numberRes++;
		}
	}

	//R2 Capacidade Máxima
	for (int i=0; i<n_linha;i++) {
		for (int j=0; j<n_colun; j++) {
			if(!Grafo[i][j]){
				model.add(x[i][j] <= Grafo[i][j]);
				numberRes++;
			}
		}
	}
	//Adicionar a outra restrição 2 do slide

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
		
		for(int i = 0; i < n_linha; i++){
			for(int j=0; j < n_colun; j++){
				if(Grafo[i][j]==0)continue;
				value = IloRound(cplex.getValue(x[i][j]));
				printf("x[%c][%c]: %.0lf\n", 65+i, 65+j, value);
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
	sum2.end();

	cout << "Memory usage before end:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	env.end();
}

int main(){

	cin >> n_linha >> n_colun;
	Grafo.resize(n_linha); //Resize matrix
	
	for(int i = 0; i < n_linha; i++){
		Grafo[i].assign(n_colun,0);
		for(int j = 0; j < n_colun; j++){
			cin >> Grafo[i][j];
			//printf("custo[%c][%c] = %d\n", 65+i, 65+j, Grafo[i][j]);
			
		}

	}
	printf("Verificacao da leitura dos dados:\n");
	printf("Tamanho da matriz: %d x %d\n", n_colun, n_linha);
	
	cplex();

    return 0;
}