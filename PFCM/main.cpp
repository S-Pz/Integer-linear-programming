
#include <bits/stdc++.h>
#include <cstdio>
#include <ilcplex/ilocplex.h>
#include <vector>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

//CPLEX Parameters
#define CPLEX_TIME_LIM 3600 //3600 segundos
int inf = std::numeric_limits<int>::max();

struct grafo{
	int origem;
	int destino;
	int custo;
	int fluxo;
};

struct Vertice{
	int cap;
	char tipo;
};

int n_arestas; //Quantidade de vertices
int n_vertices;

vector<Vertice>vertices;
vector<grafo>arestas;
vector<vector<grafo>>custo;

void cplex(){ //CPLEX
    
	IloEnv env; //Define o ambiente do CPLEX

	//Variaveis --------------------------------------------- 
	int numberVar = 0; //Total de Variaveis
	int numberRes = 0; //Total de Restricoes


	//---------- MODELAGEM ---------------

	//Definicao - Variaveis de Decisao 2 dimensoes (x_ij) não binárias (discretas)
	IloArray<IloNumVarArray> x(env);

	for(int i = 0; i < n_vertices; i++){
		x.add(IloNumVarArray(env));
		for( int j = 0; j < n_vertices; j++){
			x[i].add(IloIntVar(env, 0, CPXINT_MAX));
			numberVar++;
		}
	}

	//Definicao do ambiente modelo ------------------------------------------
	IloModel model (env);
	IloExpr sum(env); /// Expression for Sum
	IloExpr sum2(env);

	//FUNCAO OBJETIVO ---------------------------------------------
	sum.clear();
	for(int i = 0; i < n_vertices; i++ ){
		for (int j=0; j < n_vertices; j++){
			if(custo[i][j].custo == 0){
				continue;
			}
			sum += custo[i][j].custo * x[i][j];	
		}
	}
	model.add(IloMinimize(env, sum)); //Minimizacao
	
	//RESTRICOES ---------------------------------------------	
	
	//R3 Conservação de fluxo intermediários
	for(int i = 0; i < n_vertices; i++){
		
		if(vertices[i].tipo == 'o'){
			
			//printf("%d\n",i);
			sum.clear();
			for(int j = 0; j < n_vertices; j++){
				//printf("+x[%d][%d] %d\n",i,j, custo[i][j].custo );
				if(custo[i][j].custo != 0){	
					sum += x[i][j];
				}
			}

			sum2.clear();
			for(int k = 0; k < n_vertices; k++){
				if(custo[k][i].custo !=0 ){
					//printf("-x[%d][%d] %d\n",i,k, custo[i][k].custo );
					sum2 += x[k][i];
				}
			}
			model.add((sum - sum2) <= vertices[i].cap); 
			numberRes++;
		}
	}

	//R4 Capacidade Máxima
	for(int i = 0; i < n_vertices; i++){
	
		if(vertices[i].tipo == 'd'){
		
			sum.clear();
			for(int j = 0; j < n_vertices; j++){
				if(custo[i][j].custo != 0){
					sum += x[i][j];	
				}
			}

			sum2.clear();
			for(int k = 0; k < n_vertices; k++){
				if (custo[k][i].custo !=0 ){ 
					sum2 += x[k][i];
				}
			}

			model.add((sum - sum2) <= -vertices[i].cap); 
			numberRes++;
		}
	}

	for(int i = 0; i < n_vertices; i++){
		
		if(vertices[i].tipo == 't'){

			sum.clear();
			for(int j = 0; j < n_vertices; j++){
				if(custo[i][j].custo != 0){
					sum += x[i][j];
				}
			}

			sum2.clear();
			for(int k = 0; k < n_vertices; k++){
				if(custo[k][i].custo !=0 ){
					sum2 += x[k][i];
				}
			}

			model.add((sum - sum2) == 0); 
			numberRes++;
		}
	}

	//R5
	for(int i = 0; i < n_vertices; i++){
		for(int j = 0; j < n_vertices; j++){
			if(custo[i][j].fluxo != 0){
				model.add(x[i][j] <= custo[i][j].fluxo);
				numberRes++;
			}
		}
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
		
		for(int i = 0; i < n_vertices; i++){
			for(int j=0; j < n_vertices; j++){
				if(custo[i][j].custo == 0)continue;
				value = IloRound(cplex.getValue(x[i][j]));
				printf("x[%d][%d]: %.0lf\n", i, j, value);
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

	cin >>  n_vertices >> n_arestas;
	
	vertices.resize(n_vertices);
	arestas.resize(n_arestas);
	
	for (int i = 0; i < n_vertices; ++i) {
		cin >> vertices[i].tipo >> vertices[i].cap; 		
	}

	for(int i = 0; i < n_arestas; i++){
		cin >> arestas[i].origem >> arestas[i].destino >> arestas[i].custo >> arestas[i].fluxo;
	}

	custo.resize(n_vertices);

	for(int i = 0; i < n_vertices; i++){
		custo[i].resize(n_vertices);
	}
	
	// Preencher a matriz de custos e fluxos com base nas arestas lidas
    
	for (int i = 0; i < n_arestas; i++) {
        int origem = arestas[i].origem;
        int destino = arestas[i].destino;
	
        custo[origem][destino].custo = arestas[i].custo;
        custo[origem][destino].fluxo = arestas[i].fluxo;
	
    }
	printf("\n\n\n\n");

	// for(int i = 0; i < n_vertices; i++){ // For que representa o (Para Todo).
	// 	for(int j = 0; j < n_vertices; j++) {		
	// 		printf("[%d][%d] custo: %d, FluxoMax: %d\n", i,j,custo[i][j].custo, custo[i][j].fluxo);
	// 	}
	// }
	
	cplex();

    return 0;
}