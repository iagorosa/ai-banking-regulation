/*
 * Title: Financial Instability in an Agent-Based Model
 * Authors: Iago Rosa, Leonardo Goliatt, Teófilo de Paula
 * Institutions: UFJF, UFRRJ
 * License: MIT
 * Keywords: financial instability, banking regulation, credit, agent-based model, AI
 * Date: January 2025
 */

//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking

#include "fun_head_fast.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <time.h>
#include <list>
#include "kmeans.h"
#include <vector>

#define inf HUGE_VAL
//in test

using namespace std; 

const string currentDateTime() {
    time_t     now   = time(0);
    clock_t 	 clock = std::clock();
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
		 
    return buf;
}

list<int> lastTech;
void initLastTech(int tam){
	for(int i=0;i<tam;i++){
		lastTech.push_back(-1);
	}
}

void delete_matriz_tech(double **matriz_tech){
    for(int i = 0; i < 300; i++){
        delete[] matriz_tech[i];
    }
    delete[] matriz_tech;
}
// do not add Equations in this area

///////// inteligencia no c++ ///////
double **matriz_tech;
double *total_tech;
int *principal_regime_tech;
float dispersao = 0.95;
////////


auto kbank_ = 1000.0;
auto markup_ = 1.2; 
int class1_ = 3;

auto max_sem_intelig = 300; // valor maximo de iteracoes executando sem inteligencia adicionada

string tipo_inteligencia = "distancia"; // tipo de inserção da inteligencia
                                        // pode ser "classificacao" ou "distancia"

string data_ = currentDateTime();

bool save_external_files = false;
bool invalid_can_entry = true;

MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words


////////////////////
/*  TECNOLOGIA
 *  Aqui sao definido as tecnologias com diferentes parametros. 
 *  As tecnologias serao selecionadas serao selecionadas pelas diferentes firmas que estarao no sistema. Todas as tecnologias terao a mesma probabilidade de isso acontecer.
 *
 *  As variaveis nao podem estar em objetos diferentes e terem o mesmo nome. 
 *  Portanto, o sufixo t eh adicionado as variveis para informar que pertencem ao objeto tecnologia. 
 *  Para as firmas, teremos a variavel sem o sufixo. 
 */

EQUATION("At"){
/*
*   Tecnologia de produto (diferenciacao) 
*   
*/
    // O primeiro if verifica se essa tecnologia ja foi firma 
    // Caso positivo, ele tentara calcular essa funcao no primeiro passo de tempo
    // Quando ele deixa de ser firma e volta a ser tecologia, a flag Return passa a valer 1 e a funcao nao eh recalculada
    if (V("Return") == 0){
        
        if (V("ProdutoHomogeneo"))
            v[2] = 0.0;
        else{
            v[0] = V("Acoef");			//(p) - configurado no modelo
            v[1] = V("Asymmetry");	//(p) - configurado no modelo
            v[2] = norm(v[0], v[0]*v[1]);
            while ( !(v[2] > 0 and v[2] <= 10) )  // Enquanto o valor nao estiver no intervalo definido
                v[2] = norm(v[0], v[0]*v[1]);
        }
    }
    
    else{
        v[2] = VL("At", 1);
    }
        
    PARAMETER
}
RESULT( v[2] )



EQUATION("Tt"){
/*
Coeficiente tecnico de tranformacao
*/

    if (V("Return") == 0){
        if (V("ProdutoHomogeneo"))
            v[2] = 0.0;
        else{
        v[0] = V("Tcoef");			//(p) - configurado no modelo         
        v[1] = V("Asymmetry");	//(p) - configurado no modelo
        v[2] = norm(v[0], v[0]*v[1]);
        while ( !(v[2] > 0 and v[2] <= 10) )  // Enquanto o valor nao estiver no intervalo definido
            v[2] = norm(v[0], v[0]*v[1]);
        }
    }
    
    else
        v[2] = VL("Tt", 1);
    
    PARAMETER
}
RESULT( v[2] ) 


EQUATION("Ft"){
/*
Custo Fixo
*/
    if (V("Return") == 0){

        if (V("ProdutoHomogeneo"))
            v[2] = 0.0;
        else{
        v[0] = V("Fcoef");			//(p) - configurado no modelo
        v[1] = V("Asymmetry");	//(p) - configurado no modelo
        v[2] = norm(v[0], v[0]*v[1]);
        while ( !(v[2] > 0 and v[2] <= 10) )  // Enquanto o valor nao estiver no intervalo definido
            v[2] = norm(v[0], v[0]*v[1]);
        }
    }
        
    else
        v[2] = VL("Ft", 1);
    
    PARAMETER

}
RESULT( v[2] )


EQUATION("Save_Tech_Information"){

    std::string expec = "";
    if V("ExpectativaHomogenea")
        expec = "_homo";
    else
        expec = "_hetero";
        

    data_ = currentDateTime();
    std::string entry_ = "";
    if V("entry")
        entry_ = "_entry";
    else
        entry_ = "_no_entry";
    
    std::ofstream fout;
    
    fout.open("resultados/tech_inf"+expec+entry_+"_"+data_+".csv",ios::out);
    
    CYCLE(cur, "Tech"){
        fout << VS(cur, "idT") << "," << VS(cur, "At") << "," << VS(cur, "Tt") << "," << VS(cur, "Ft") << endl;
    }
    
    fout.close();
}
RESULT( 1 )

///////////////////

///////////////////////////////////
////// DEFINICOES INICIAIS - START /////////
EQUATION("Init_Tech"){
/*
* Inicializacao das tecnologias definidas acima
* Esta eh apenas uma funcao dentro do modelo
* Funcao inserida no agente Tech
* Funcao technology no NetLogo
*/
	
	// Variavel Global para informacao de produto homogeneo 
    int id = 0;
	CYCLE(cur, "Tech"){
        WRITES(cur, "idT", id++);
        WRITES(cur, "Return", 0);
		VS(cur, "At");
		VS(cur, "Tt");
		VS(cur, "Ft");	
    }

    if (save_external_files) V("Save_Tech_Information");

    int qtd_tech = (int) COUNT("Tech");
    matriz_tech = new double*[qtd_tech];
    for (int i = 0; i < qtd_tech; i++) {
        // Class2 = 0, Class2 = 1, Class2 = 2, soma das 3 primeiras colunas, principal Class2
        matriz_tech[i] = new double[5];
        for (int j = 0; j < 5; j++)
            matriz_tech[i][j] = 0.0;
    }

    PARAMETER
}
RESULT( 1 )


EQUATION("Init_Firms"){
/*
 * Inicia as firmas com os parametros da tecnologia selecionada
 * Nas firmas, as variaveis construidas sao associadas pelas variaveis de mesmo nome + sufixo "t" do objeto tecnologia.
 * No final, a tecnolia selecionada eh apagada
 * Em relacao ao NetLogo, essa parte eh referente a selecao de uma tenologia e set de firma (onde repete 100 vezes)
 */
    markup_ = V("Mk_init"); 
    int id = 0;
    CYCLE(cur, "Firm"){
        WRITES(cur, "id", ++id);             // Cricao de ids para as firmas.
        
        cur1 = RNDDRAW_FAIR( "Tech" );      // seleciona uma teclogia dentre as existentes com a mesma probabilidade.
        
        v[0] = VS(cur1, "At");
        WRITES(cur, "A", v[0]);
        
        v[1] = VS(cur1, "Tt");
        WRITES(cur, "T", v[1]);
        
        v[2] = VS(cur1, "Ft");
        WRITES(cur, "F", v[2]);
        
        v[3] = VS(cur1, "idT");
        WRITES(cur, "idTech", v[3]);
        
        // Inicializacao de entrada e saida das firmas no sitema
        WRITES(cur, "entrada", 0);
        WRITES(cur, "saida", -1);  // inicializado com -1 
        
        DELETE(cur1);
    }
    
    // BUSCA A INFORMACAO DOS SALARIOS OU PRECOS DE INSUMO
    v[2] = V("W"); // Salario ou preco de insumos
    
    // Busca da quantidade de firmas no modelo inicialmente
    v[4] = V("Qtd_firmas");
    
    // DEFINICOES DAS VARIAVEIS DAS FIRMAS
    CYCLE(cur, "Firm"){
        
        if (V("ExpectativaHomogenea")) {
            v[0] = 3.0;  // t3
            // WRITES(cur, "Class1", class1_);   // escreve Class1 (tipo de expectativa)
        }
        else{
            v[0] = 1.1 + uniform(0, 8.9);
            v[1] = (int)uniform(0, 8);
            WRITES(cur, "Class1", v[1]);
            }
        
        // VARIAVEIS COM ALEATORIEDADE
        v[1] = VS(cur, "F");   // custo fixo e escala mínima
        /////////
        
        // Capital minimo (Kmin) de cada firma
        v[3] = v[0] * v[1] * v[2];  // v[2] = V("W") = Salario ou preco de insumos definido acima
        WRITES(cur, "Kmin", v[3]);  // Forca a escrita do capital minimo de cada firma
        ///////
                
        for (int i = 1; i <= 10 + 1; i++)   // condição precisa ser o valor desejado + 1 por algum motivo
            WRITELS(cur, "Qd", v[3], t-i); // Escreve em Qd no tempo t-i o valor de Kmin 
        
        WRITELS(cur, "M", 1.0/v[4], t-1);  // Definicao do Market Share igual para todos no comeco
        
        WRITELS(cur, "V", 0.0, t-1);       // Definicao dos estoques iniciais iguais a 0 (zero)
               
        // Definicao do markup
        WRITELS(cur, "Mk", markup_, t-1);           // markup
        ///////       
        
        // Lucros acumulados no comeco sao iguais a zero
        WRITELS(cur, "AProfit", 0.0, t-1);
        
        WRITELS(cur, "ADebit", 0.0, t-1);
        WRITELS(cur, "Sd", 0.0, t-1); 
    }
    
    WRITE("max_firm_id", v[4]-1);
    
    PARAMETER
    
}
RESULT(1)


EQUATION("Init_System"){
/*   
 *  Executa apenas a primeira vez 
 *  Calcula e escreve o captal minimo de cada firma (Kmin)
 *  Calcula e escreve o Qd de cada firma em 10 passos de tempos para tras.
 *  O valor escrito eh o valor de Kmin para todos os passos
 *  NETLOGO: start
 */ 

    v[0] = (int) V("tamLastTech");
    initLastTech(v[0]);
    
    // DEFINICAO DO TEMPO
    WRITE("time", 0);
    
    // Capital inicial do banco
    WRITEL("Kbank", kbank_, t-1);						// capital do banco
    
    
    // Inicializacao valores de g 
    for (int i = 1; i <= 20 + 1; i++)   // condição precisa ser o valor desejado + 1 por algum motivo
        WRITEL("h", 1, t-i);
    
    WRITE("LastTech", -1);

    PARAMETER // Transforma o Init em parametro para que execute apenas uma vez de fato

}
RESULT (1)

//////// RUN WORLD //////////
EQUATION("run_world"){
/*
 * Funcao equivalente ao run-world do NetLogo
 * Serve para organizar as chamadas das funcoes e variaveis
 * Definir como funcao do sistema
 */
    
    V("Qtd_firmas");
    
    if (V("entry") && V("time") > 0 && COUNT("Tech") > 0){
        V("Entry_Func");
    }
        
    V("Vtotal");                // Estoques totais
    V("CreditMarket");          // Expectation + CreditMarket
    
    V("Limit_Kbank");                 // Banco define o volume de capital a ser emprestado, dada a demanda total por crédito
    
    V("Production");                 // Funcao producao para todas as firmas. A funcao tem um loop para todas
    
    V("Price_Competitiveness");      // Definicao de preco e competitividade para todas as firmas
    
    V("Global_Competitiveness");    // Calculo da competitividade media
     
    V("MarketShare");               // Calcula o MarketShare de cada firma
    
    V("Price_Level");               // Calcula o preco medio praticado pelas empresas (mercado)
    
    V("Income_Government");        // Renda, governo e tributação
    
    V("Sales_Profit");             // Receita, estoque e renda não gasta 
    
    V("Markup");                   // Valor de markup
    
    V("FinancingRegime");          // Definicao do tipo de regime das firmas
    
    V("BankBehaviour");            // Ajuste no spread
    
    if (save_external_files)
        V("Save_Information");          // Salva as informacoes que serao necessarias para analise
                                        // Ultima parte do codigo
    
    if (V("time") >= 1 && ( V("inteligencia") || V("inteligencia_sempre") )) 
        V("Incrementa_Matriz_Tech");

    V("time");                     // contador de tempo 

}
RESULT (1)

EQUATION("Inteligencia_Entry"){
/*
 * Funcao que insere inteligencia na decisao do banco em qual Tecnologia escolher.
 * O codigo da decisao esta em python 
 */ 
        if (V("time") >= 1)
            v[1] = V("menor_distancia_kmeans");
}
RESULT( v[1] )


EQUATION("Entry_Func") {
/*
 * Função para entrada de novas firmas
 * Definir com funcao do sistema
 */
    v[0] = uniform_int( 1, 10 );
    cur1 = RNDDRAW_FAIR( "Tech" ); // seleciona uma teclogia dentre as existentes com a mesma probabilidade.
    if ( ((V("time") >= max_sem_intelig && V("inteligencia")) || V("inteligencia_sempre")) && v[0] <= V("random_threshold") ) {
        v[0] = V("Inteligencia_Entry");
        LOG("id a procurar: %g\n", v[0]);
        if (v[0] != -1)
            cur1 = SEARCH_CND( "idT", v[0]); 
    }
    // else
        
    if (v[0] != -1 || invalid_can_entry) {
        
        cur2 = ADDOBJL("Firm", t-1);
        
    //     int max_id = MAX("id");
        int max_id = V("max_firm_id");
        WRITES(cur2, "id", max_id+1);
        WRITE("max_firm_id", max_id+1);
        
        // Pegando variaveis de Tech e colocando na Firm
        v[0] = VS(cur1, "At");
        WRITES(cur2, "A", v[0]);
            
        v[1] = VS(cur1, "Tt");
        WRITES(cur2, "T", v[1]);
            
        v[2] = VS(cur1, "Ft");
        WRITES(cur2, "F", v[2]);
        
        v[3] = VS(cur1, "idT");
        WRITES(cur2, "idTech", v[3]);


        // Variaveis abaixo sao necessarias pois dependem do valor no tempo passado durante a execucao do codigo    

        //// INFORMCOES QUE DEPENDEM DE ESTAR NO TEMPO t-1
        // Lucros acumulados no comeco sao iguais a zero
        WRITELS(cur2, "AProfit", 0.0, t-1);
        // Debitos no comeco sao iguais a zero
        WRITELS(cur2, "ADebit", 0.0, t-1);
        // Saldo devedor
        WRITELS(cur2, "Sd", 0.0, t-1); 
        // Estoques
        WRITELS(cur2, "V", 0.0, t-1);      

        ////INFORCOES QUE PRECISAM SER INICIALIZADAS
        v[4] = V("Qtd_firmas");            // +1 necessario pois considera a firma que esta entrando agora     
        WRITELS(cur2, "M", 1.0 / v[4], t-1);    
        WRITELS(cur2, "Mk", markup_, t-1);
        
        if (V("ExpectativaHomogenea")) {
            v[5] = 3.0;  // t3
            WRITES(cur2, "Class1", class1_);   // escreve Class1 (tipo de expectativa)
        }
        else {
            v[5] = 1.1 + uniform(0, 8.9);
            v[6] = (int)uniform(0, 8);
            WRITES(cur2, "Class1", v[6]);
        }     
        
        v[7] = V("W");         // Salario ou preco de insumos
        
        // Capital minimo (Kmin) de cada firma
        v[8] = v[5] * v[2] * v[7];          // Kmin = v[5]FW
        WRITES(cur2, "Kmin", v[8]);         // Forca a escrita do capital minimo de cada firma
        
        v[9] = v[8] / v[7];                 // L (Insumos) = Kmin / W
        
        v[10] = v[1] * (v[9] - v[2]);        // Qs = T(L-F)
            
            
        for (int i = 1; i <= 10 ; i++) 
            WRITELLS(cur2, "Qd", v[8], t-1, i);
        
        v[11] = ( markup_ * v[8] * (1.0 + VL("r", 1)) ) / v[10]; // P
        
        WRITES(cur2, "entrada", V("time") + 1); // tempo em que a firma entrou no sistema
        WRITELS(cur2, "P", v[11], t-1);
        
        if ( v[11] > VL("Pbank", 1)){
            cur3 = ADDOBJ("Tech");
            WRITES(cur3, "At", VS(cur2, "A"));
            WRITES(cur3, "Tt", VS(cur2, "T"));
            WRITES(cur3, "Ft", VS(cur2, "F"));
            WRITES(cur3, "idT", VS(cur2, "idTech")); // volta com o indice original
            WRITES(cur3, "Return", 1);   // marca essa tecnologia como retornada ao ao objeto Tech
                                        // Isso faz com que a os parametros de Tech nao sofram atualizacao
            DELETE(cur2);
        }
        DELETE(cur1);
    }

}
RESULT( 1 )  


//////// CREDIT MARKET //////////
/// Expectation aqui dentro
EQUATION("CreditMarket") {
/*
 * Definicoes das equacoes definidas em CreditMarket no NetLOGO.
 * Definir como funcao da firma 
 */
    CYCLE(cur, "Firm"){      
        VS(cur, "E");         // Expectation
        VS(cur, "Qe");        // Quantidade esperada

        VS(cur, "LF");       // Calcula os insumos 
        VS(cur, "Wl");       // Calcula os insumos pelo seus precos 
            
        // Definicoes abaixo referente as Eq. 5
        VS(cur, "KLD");    
    }
}
RESULT(1)


//////// LIMIT KBANK //////////
EQUATION("Limit_Kbank"){
/*
 * Definicoes das equacoes de Limit_Kbank no NetLOGO.
 * Definir como funcao do sistema
 * A equacao verifica o capital do banco e quanto eh o montante separado para aplicacaoem titulos.
 * se a demanda por credito for maior que a capital disponivel, ajustes sao realizados (inclusive na equacao Production, onde os emprestimos das firmas sao de fato ajustados)
 */
    V("KLDtotal");           // Demanda de credito total considerando todas as firmas
    V("KtitM");              // Montante minimo do banco aplicado em titulos publicos 
                             // valor alterado posteiormente na variavel Ktit caso o banco nao tenha dinheiro suficiente para arcar com toda demanda por credito
                             // Nesses casos, o valor de Ktit eh sobrescrito somando o valor minimo calculado com a diferenca do capital do banco pela demanda de credito das firmas.       

    V("red");                // Parametro sobre reducao de credito calculado acima - parametro do sistema
                             // Sera necessario em Production
    
    V("Dc");
    V("Ktit");
    V("Cred");              // Volume de credito efetivamente concedido as firmas
}
RESULT (1)  


EQUATION("Production"){ //4
/* 
 * Definicao das equacoes de Production do NetLogo
 * Definir como funcao do sistema
 * Aqui acontecera a redução, caso necessario, da demanda por credito de cada firma de acordo com o valor da redução calculado em Limit_Kbank
 * A transferencia de recursos do banco para as empresas acontecera aqui efetivamente
 * O loop das firmas esta acontecendo aqui e nao no run-world.
*/

    v[5] = V("W");              // Salario ou precos dos insumos

    CYCLE(cur, "Firm"){         // Procura para todas as firmas
        
        VS(cur, "KL");
        VS(cur, "K");
        VS(cur, "AProfitD");
        VS(cur, "SdL");
        VS(cur, "ADebitB");
        VS(cur, "L");        
    }
    
    V("KLtotal");                               // Total de emprestmo concedido
    V("KbankT");                                // Desconta todos os emprestimos solicitados e atentendidos do capital do banco
    V("Wtotal");                                // massa salarial  
    
    V("Kfail");                                 // emprestimos nao honrados
    
    CYCLE(cur, "Firm"){
        VS(cur, "Qs");  
    }

}
RESULT(1)


EQUATION("Price_Competitiveness"){ //5
/*
 * Aqui serao calculados os precos dos produtos e a competitividade
 * Funcao do sistema que chamara para cada firma
 * O loop das firmas esta acontecendo aqui e nao no run-world.
 */
    CYCLE(cur, "Firm"){ 
    
        VS(cur, "P");           // Definicao dos precos praticados por cada firma
        VS(cur, "C");            // Definicao da competitividade de cada firma

    }

}
RESULT(1)


EQUATION("Global_Competitiveness"){ //6
/*
 * Calcula a competitividade media das firmas
 * Referente a Eq. 11
 * Funcao do sistema
 */
    V("Cmed");  
//     WRITE("Cmed", 1);

}
RESULT(1) 


EQUATION("MarketShare"){ //7
/*
 * Calcula o market share de cada uma das firmas do sistema
 * Referente a funcao MarketShare do netlogo
 * Referente a Eq. 10 do artigo 
 * Funcao do sistema
*/
    
    CYCLE(cur, "Firm"){
        VS(cur, "MA");          // Marketshare inicial/bruto
    }

    // Normaliza os valores de M entre 0 e 1
    CYCLE(cur, "Firm"){ 
        VS(cur, "M");            // Market share normalizado
    }
}
RESULT(1)


EQUATION("Price_Level"){ //8
/*
 * Calcula o preço medio praticado pelas firmas
 * Serao considerados aqui duas diferentes variaveis:
 *      firmas apenas Hedge e todas as firmas
 * Funcao do sistema
 * Funcao Price-Level no NetLOGO
 */

    V("Pmed");       // Pmed eh o preco medio determinado somente por firmas Hedge
    V("PmedGeral");  // preco medio detrminado por todas as firmas

}
RESULT( 1 )


EQUATION("Income_Government"){ // 9
/*
 * Renda, governo e tributacao
 * Equacao Income-Government do netlogo
 * Referente a Eq. 13 do artigo
 */
    
    V("Y");                    // Rendimento dos titulos 
    V("beta");
    V("Yg");                     // Renda disponivel
    V("Yd");
    
    // Nao eh necessario zerar nem Wtotal e nem S, como o modelo netlogo sugere.
    // A cada tempo as variaveis sao resetadas aqui no lsd 

}
RESULT( 1 )


EQUATION("Sales_Profit"){ // 10
/*
 * Considera os diferentes cenarios para calcular R, S e V
 * Quantidade demandada (Qs) tambem eh calculada aqui 
 * Ha uma possivel troca de estrategia de demanda esperada. Atencao aqui.
 * Referente a equacao Sales-Profit no netlogo
 * Referente as Eqs. 14-16 no artigo
 * Funcao do sistema
 */

    CYCLE(cur, "Firm"){
        
        VS(cur, "Qd");            // Quantidade demandada        
        VS(cur, "R");                   // Receita de vendas
        VS(cur, "V");                   // Estoques
    }
    
    V("S");                            // Renda nao gasta de fato
    
}
RESULT(1)


EQUATION("Markup"){ //11
/*
 * Calcula o valor de markup de cada firma a cada periodo de tempo
 * Funcao do sistema
 */

    CYCLE(cur, "Firm"){
        VS(cur, "Mk");
    }

}
RESULT(1)


EQUATION("FinancingRegime"){ //12
/*
 * Faz as verificacoes e calculos dos regimes financeiros
 * Calcula os recursos totais, juros e amortização de divida das firmas
 * Referente a equacao FinancingRegime do NetLogo
 */

    CYCLE ( cur, "Firm" ) {
        
        // Direciona os calculos de AProfit, J, AM e Class2 de acordo com o tipo de firma

        VS(cur, "Rtot");                    // Recursos totais da firma                        
        
        VS(cur, "Class2");
        
        VS(cur, "AProfit");                 // Lucros acumulados
        VS(cur, "J");                       // Juros
        VS(cur, "AM");                      // Amortizacao

        VS(cur, "ADebit");                  // Dividas acumuladas
        VS(cur, "Sd");                      // creditar a amortizacao na dívida        
    }
    
    V("JAMe");                              // Juros e amortização esperados.
    V("JAMf");                              // Juros e amortização efetivados.
    V("Jtot");                              // Juros recebidos
    
    V("NumHedge");                          // contador de firmas regimes tipo Hedge
    V("NumSpec");                           // contador de firmas regimes tipo Especulativo
    V("NumPonzi");                          // contador de firmas regimes tipo Ponzi
    
    V("NumHedge_Pct");                          // % de firmas regimes tipo Hedge
    V("NumSpec_Pct");                           // % de firmas regimes tipo Especulativo
    V("NumPonzi_Pct");                          // % de firmas regimes tipo Ponzi

}
RESULT (1)


EQUATION("BankBehaviour"){ //13
/*
 * Ajuste no spread
 * Faz os calculos e ajustes referentes ao banco
 * Funcao de mesmo nome no NetLogo 
 * Funcao do sistema
*/
    V("h");                                // rendimento líquido de aplicações privadas em relação a rendimento em títulos públicos
    V("g");                                // Desvio padrao de h
    V("Pbank");                            // Preco limite estabelicido pelo banco
    V("Frag");                             // rendimentos esperados - rendimentos recebidos / rendimentos esperados
    V("spread");                           // Spread bancario 
    V("r");                                // Juros
    V("sigma");                            // Taxa desejada em aplicacao em titulos
    V("AProfitTotal");                     // Soma de todos os valores de AProfit
    V("Kbank");                            // capital do banco
    V("Wealth");                           // Saude do banco - Riqueza total da economia

}
RESULT(1)


//////////////////////////////


//////////// EQUACOES  ///////////////
EQUATION("AM"){
/*
 * Amortização da divida
 * Calculo da amortização da divida referente a cada tipo de sistema financeiro
 * Referente a Eq. 20
*/
    v[0] = V("Class2");             // Verifica o tipo de regime financeiro que a firma se encontra
    
    v[1] = V("Rtot");               // Receita total
    v[2] = VL("r", 1);              // taxa de juros de mercado
    v[3] = V("ADebitB");            // Dividas acumuladas brutas/iniciais
    
    if (v[0] == 1)
        v[4] = v[3];
    else{
        if(v[0] == 2)
            v[4] = v[1] - v[2] * v[3];
        else
            v[4] = 0.0;
    }

}
RESULT(v[4])


EQUATION("AProfit"){
/*
 * Lucros acumulados. 
 * Calculados de fato em FinancingRegime
*/ 
    
    v[0] = V("Class2");             // Verifica o tipo de regime financeiro que a firma se encontra
                                    // 1 = hedge, 2 = especulativo, 3 = ponzi
    
    v[1] = V("Rtot");               // Receita total
    v[2] = VL("r", 1);              // taxa de juros de mercado
    v[3] = V("ADebitB");            // Dividas acumuladas brutas/iniciais
        
    v[4] = 0.0; 
    if ( v[0] == 1 )
        v[4] = v[1] - v[3] * (1 + v[2]);
}
RESULT(v[4])


EQUATION("AProfitTotal") {
/*
 * Soma de todos os valores de AProfit das firmas 
*/ 
    v[0] = SUM("AProfit");
    
}
RESULT(v[0])


EQUATION("AProfitD"){
/*
 * Lucro das firmas considerando pagamento de despesas
*/
    v[0] = VL("AProfit", 1);
    v[1] = V("Wl");
    
    v[2] = v[0] - v[1];
    if ( v[0] < v[1] )
        v[2] = 0;
}
RESULT( v[2] )



EQUATION("ADebit"){
/*
 * Dividas acumuladas. 
 * Calculados de fato em FinancingRegime
*/        
    v[0] = V("ADebitB");            // Dividas acumuladas brutas/iniciais
    v[1] = VL("r", 1);              // taxa de juros de mercado
    v[2] = V("J");                  // Juros
    v[3] = V("AM");                 // Amortização

    v[4] = v[0] * (v[1] + 1) - v[2] - v[3];
    
    if (v[4] < 0) v[4] = 0.0;       // Restrição caso ADebit fique negativo
}
RESULT(v[4])


EQUATION("ADebitB"){
/*
 * Dividas acumuladas brutas/iniciais
*/     
    v[0] = VL("ADebit", 1);               // Receita total
    v[1] = V("KL");                       // taxa de juros de mercado

    v[2] = v[0] + v[1];

}
RESULT(v[2])


//// Price-Competitiveness
EQUATION("C"){
/*
 * Defincao da competitividde da firma
 * Calulo da competitividade de cada firma. A equacao eh chamada dentro do Price_Competitiveness
 */
    v[1] = V("P");                              // Preco calculado ainda no loop do Price_Competitiveness
    v[2] = VL("M", 1);                          // Market Share da firma no período anterior
    v[3] = V("A");                              // Tecnologia de produto da empesa
    
    if (v[1] * pow(v[2], v[3]) > 0.0000001)
        v[0] = 1.0 / (v[1] * pow(v[2], v[3]));
}
RESULT(v[0])


//// Global-Competitiveness
EQUATION("Cmed"){
/*
 * Competitividade media
 * Eq. 11
 * Variavel do sitema
*/
    v[0] = 0.0;
    CYCLE(cur, "Firm") //Procura dentro das firmas os valores de Market Share (M)
                    //E os valores da competitividade (C). Entao, multiplica-os
    {
        v[1] = VLS(cur, "M", 1);	// Market share (v)
        v[2] = VS(cur, "C");	    // Competitividade (v)
        v[0] += (v[1]*v[2]);
    }

}
RESULT(v[0])


EQUATION("Cred"){
/*
 * Volume de recursos disponivel para emprestimo
 * Chamado em Limit_Kbank 
 * Criado como variavel para acompanhar a sua evolução
 */
    v[0] = VL("Kbank", 1);
    v[1] = V("Ktit");
    
    v[2] = v[0] - v[1];

}
RESULT( v[2] )


EQUATION("Dc"){
/*
 * Demanda por credito total (todas as firmas) ajustada pelo redutor 
 */
    
    v[0] = V("KLDtotal");
    v[1] = V("red");
    
    v[2] = v[0] * v[1];   
}
RESULT( v[2] )


EQUATION("E"){
/*
 * Demanda esperada
 * Programado as 8 opcoes propostas pelo Teo
 * Atencao a essa parte pois aqui pode entrar algo novo
 * NETLOGO : Expectation e CreditMarket
*/
    v[0] = V("Class1");
    
    if (v[0] == 0 || v[0] == 5 ){ // Media(Qd)
        int tam = (v[0] == 0) ? 10 : 5;
        v[1] = 0.0;
        for (int i = 1; i <= tam; i++){
        }
        v[1] = (float) v[1] / tam;
    }else{
        if (v[0] == 1 || v[0] == 6){ // MAX(Qd)
            int tam = (v[0] == 1) ? 10 : 5;
            v[1] = VL("Qd", 1);
            for (int i = 2; i <= tam; i++){
                v[2] = VL("Qd", i); 
                if (v[2] > v[1])
                    v[1] = v[2];
            }
        }else{
            if (v[0] == 2 || v[0] == 7){ // MIN(Qd)
                int tam = (v[0] == 2) ? 10 : 5;
                v[1] = VL("Qd", 1);
                for (int i = 2; i <= tam; i++){
                    v[2] = VL("Qd", i); 
                    if (v[2] < v[1])
                        v[1] = v[2];
                }
            }else{
                if (v[0] == 3){ // MEDIAN(Qd)
                int tamanho = 10;
                double *ord = new double[tamanho];
                for(int i = 0; i < tamanho; i++) ord[i] = VL("Qd", i+1);

                sort(ord, ord + tamanho);
                if(tamanho%2)
                    v[1] = ord[(int)tamanho/2];
                 else 
                    v[1] = (ord[tamanho/2-1]+ord[tamanho/2])/2;

                }else{
                    if (v[0] == 4){
                        v[1] = VL("Qd", 1);
                    }
                }
            }
        }
    }    
}
RESULT( v[1] )


EQUATION("Frag"){
/*
 * rendimentos esperados - rendimentos recebidos / rendimentos esperados
 * Substitui Frag1 do NetLogo
 * Equacao em BankBehaviour
 * Equivalente a Eq. 31 - variavel H do modelo
*/ 
    // Equivalente a Frag1
    v[0] = V("JAMe");                // g7
    v[1] = V("JAMf");                // g8

    if (v[0] != 0)
        v[2] = (v[0] - v[1]) / v[0];     // Frag1
    
    if (v[2] < 0.05 || v[0] == 0) v[2] = 0.05;   // Limite inferior

}
RESULT (v[2])


EQUATION("g"){
/*
 * Desvio padrao da memoria do banco
 * Modelei o problema sem criar uma memoria propriamente dita 
*/ 
   
    /*
     * Verifica se tem pelo menos 2 elementos de h calculados.
     * Caso nao haja, considera g = 1
     * Caso haja, verifica se o tempo atual eh maior do que o valor estabelicido por memoH 
     *      Se nao for, o for tera tamanho "time"
    */
    
    int tam = (int) V("memoH");
    v[0] = V("h");
    for (int i = 1; i < tam; i++)
        v[0] += VL("h", i);
    v[0] /= tam; // valor da media
        
    v[1] = pow( (V("h") - v[0]), 2 );
    for (int i = 1; i < tam; i++)
        v[1] += pow( (VL("h", i) - v[0]), 2 );
    v[1] /= (tam - 1); // desvio-padrao
    
    v[0] = sqrt(v[1]);    

}
RESULT(v[0])


EQUATION("h"){
/*
 * Rendimento líquido de aplicações privadas em relação a rendimento em títulos públicos
 * Equacao em BankBehaviour 
*/
    v[0] = V("Jtot");                          // Juros totais - g10 no NetLogo
    v[1] = V("Kfail");                         // Emprestimos nao honrados
    v[2] = V("Cred");                          // Volume de credito concedido 
    v[3] = V("Yg");                            // Rendimento dos titulos
    v[4] = V("Ktit");                          // Capital do banco em titulos publicos
    
    if(v[2] == 0 || v[3] == 0)
        v[5] = 1;
    else
        v[5] = ( (v[0] - v[1]) / v[2] ) * ( v[4] / v[3] ); 
}
RESULT( v[5] )


EQUATION("J"){
/*
 * Juros
 * Calculo dos juros dependendo do regime financeiro de cada firma
 * Referente a Eq. 19
*/
    v[0] = V("Class2");             // Verifica o tipo de regime financeiro que a firma se encontra
    
    v[1] = V("Rtot");               // Receita total
    v[2] = VL("r", 1);              // taxa de juros de mercado
    v[3] = V("ADebitB");            // Dividas acumuladas brutas/iniciais
    
    if (v[0] == 3)
        v[4] = v[1];
    else
        v[4] = v[2] * v[3];
}
RESULT(v[4])


EQUATION("JAMe"){
/*
 * Juros e amortização esperados.
 * Equacao de aopoio 
 * Substitui g7 do NetLogo
 * Equacao do sistema
*/ 
    v[1] = VL("r", 1);                 // taxa de juros de mercado
    v[2] = SUM("ADebitB");              // Total de debitos das firmas

    v[4] = (1 + v[1]) * v[2];
    
}
RESULT(v[4])


EQUATION("JAMf"){
/*
 * Juros e amortização efetivados.
 * Equacao de aopoio 
 * Substitui g8 do NetLogo
 * Equacao do sistema
*/ 
    v[0] = 0.0;
    CYCLE(cur, "Firm"){
        v[1] = VS(cur, "J");                  // Juros
        v[2] = VS(cur, "AM");                 // Amortização
        
        v[0] += (v[1] + v[2]);
    }
}
RESULT(v[0])


EQUATION("Jtot"){
/*
 * Juros recebidos.
 * Equacao de aopoio 
 * Substitui g10 do NetLogo
 * Equacao do sistema
*/ 
    v[0] = SUM("J");
}
RESULT(v[0])


EQUATION("K"){
/*
 * Montante de capital para investimento
Eq. 6
*/

    v[0] = V("Wl");                             //Despesas de produção dada a expectativa de demanda (v)
    v[1] = VL("AProfit", 1);                    //Parcela de A^p usada para custear producao (A^P*) - No estagio atual, eh equivalente a AProfit. 
    
    v[3] = v[0];
    if(v[1] < v[0]){                           // Se os lucros acumulados forem menores do que as despesas de producao, entao:
        v[2] = V("KL");
        v[3] = v[2] + v[1];
    }
}
RESULT (v[3])


EQUATION("Kbank"){
/*
 * Referente ao ultimo calculo de Kbank no modelo NetLogo
 * Chamado em BankBehaviour
 */

    v[0] = V("Ktit");                       // Capital do banco em titulos publicos
    v[1] = V("Yg");                         // Rendimento dos titulos
    v[2] = V("JAMf");                       // Juros e amortização efetivados
    
    v[3] = v[0] + v[1] + v[2];
    
    if(v[3] > 950) v[3] = 900 + (int(V("Ktit")) % 10);
}
RESULT (v[3])


EQUATION("KbankL"){
/*
 * Kbank intermediario
 * Capital do banco disponível para emprestimo
 */

    v[0] = VL("Kbank", 1);                     // Capital do banco no ultimo periodo
    v[1] = V("KtitM");                         // Rendimento dos titulos
    
    v[2] = v[0] - v[1];

}
RESULT (v[2])


EQUATION("KbankT"){
/*
 * Capital do banco após concessao dos empresetimos para as firnas
 */

    v[0] = V("Cred");                     // Volume de recursos disponivel para emprestimo
    v[1] = V("KLtotal");                   // Credito total efetivamente concedido pelo banco referente a todas as firmas
    
    v[2] = v[0] - v[1];

}
RESULT (v[2])


EQUATION("Kfail"){
/*
 * Empréstimos não honrados (somatório de SDL - saldo devedor considerando emprestimo pego)
 * variavel do sistema
 * Se uma firma não consegue pagar os emprestimos tomados, ela quebrará (vira Tech novamente) e o valor que devia entra na conta dos emprestimos nao honrados do banco (Kfail).
 */
    
    v[0] = 0.0;
    CYCLE_SAFE(cur, "Firm"){   // CYCLE_SAFE eh necessario para conseguir deletar o objeto dentro do laço 
        
        // Variaveis necessarias para comparacao abaixo
        v[1] = VS(cur, "L");
        v[2] = VS(cur, "F");
        v[3] = VLS(cur, "V", 1);
        
        // Verificacao se os insumos cobrem o curto e se tem estoques. 
        // Caso isso não ocorra, a firma em questao quebra e volta a ser uma tecnologia.
        if (v[1] < v[2] || (v[1] == v[2] && v[3] == 0.0)){
            v[4] = VS(cur, "SdL");
            v[0] += v[4];
            
            v[5] = VS(cur, "T");
            v[6] = VS(cur, "A");
            v[7] = VS(cur, "idTech");
            
            cur1 = ADDOBJ("Tech");
            WRITES(cur1, "At", v[6]);
            WRITES(cur1, "Tt", v[5]);
            WRITES(cur1, "Ft", v[2]);
            WRITES(cur1, "idT", v[7]);
            WRITES(cur1, "Return", 1);   // marca essa tecnologia como retornada ao ao objeto Tech
                                         // Isso faz com que a os parametros de Tech nao sofram atualizacao

            if V("zerarFail"){
                for(int i=0; i<3; i++)  matriz_tech[ (int) v[7] ][i] = 0.0;
            }

            DELETE(cur);
        }
    }    
}
RESULT(v[0])


EQUATION("KL"){
/*
 *  Firma ajusta demanda por credito (KLD) ofertado pelo banco
 */ 
    v[0] = V("KLD");
    v[1] = V("red");

    v[2] = v[0] * v[1];         // Aplica a reducao calculada em Limit_Kbank ao emprestimo solicitado por cada firma
}
RESULT(v[2])


EQUATION("KLtotal"){
/*
 * Credito total efetivamente concedido pelo banco referente a todas as firmas
*/
    v[0] = SUM("KL");
}
RESULT (v[0])


EQUATION("KLD"){
/* 
 * Emprestimo demandado pela firma pelo banco
 * Eq. 5
*/

    v[0] = V("Wl");								//Despesas de produção dada a expectativa de demanda (v)
    v[1] = VL("AProfit", 1); 	                //Parcela de A^p usada para custear producao (A^P*) - No estagio atual, eh equivalente a AProfit. 
                                                
    v[2] = 0.0;    
    if (v[1] < v[0])
        v[2] = v[0] - v[1];
}
RESULT(v[2])


EQUATION("KLDtotal"){
/*
 * Demanda de credito total referente a todas as firmas
 * Variavel g11 do NetLOGO em Limit_Kbank
*/
    v[0] = SUM("KLD");
}
RESULT (v[0])


EQUATION("Ktit"){
/*
 * Definicao do capital disponivel para emprestimo
 * Eq. 37 do modelo - X_t^B
 */
    v[0] = V("Dc");                 // Demanda por credito total (todas as firmas) ajustada pelo redutor 
    v[1] = V("KtitM");              // Montante minimo para aplicacao em titulos
    v[2] = V("KbankL");             // Capital do banco disponível para emprestimo
    
    if (v[2] > v[0])                // Verifica se o capital disponivel para emprestimo (v[3]) eh maior que a demanda por credito ajustada pelo redutor (v[4])
                                    // Sendo assim, estaria "sobrando" dinheiro para o banco e ele faria aplicação em titulos publico desse valor a mais
        
        v[1] += (v[2] - v[0]);       // soma a Ktit a diferenca entre Kbank e ajuste de credito pelo redutor (g12) - caso possivel apenas quando nao há redução
                                    // Se o capital do banco arcar com os emprestimos e ainda sobrar dinheiro, o valor eh aplicado em titulos publicos
}
RESULT( v[1] )


EQUATION("KtitM"){
/*
 * Montante minimo para aplicacao em titulos
*/

    v[0] = VL("sigma", 1);    // parametro que define a aplicacao minima
    v[1] = VL("Kbank", 1);
    v[2] = v[0] * v[1];

}
RESULT (v[2])


EQUATION("L"){
/*
 * firma adquiri insumos 
*/
    v[0] = V("K");
    v[1] = V("W");
    
    v[2] = v[0] / v[1];
}
RESULT ( v[2] )


EQUATION("LF"){
/*
 * Insumos (trabalhadores, horas de trabalho)
 * Eq. 4
 * NETLOGO: CreditMarket
*/
    v[0] = V("Qe");	//Producao eperada pela firma (v)
    v[1] = V("F");	//Custo fixo(p)
    v[2] = V("T");	//Coeficiente tecnico de tranformacao (p)

    v[3] = v[0] / v[2] + v[1];

}
RESULT( v[3] )


EQUATION("MA"){
/*
 * Market share bruto das firmas
 * Referente a Eq. 10 do artigo 
 * Os valores ainda precisam ser normalizados.
 */

    v[0] = V("Cmed");       // Competitividade media
    v[1] = V("lambda");     // Parametro - velocidade de ajustamento de M 
    
    v[2] = VL("M", 1);
    v[3] = V("C");
    
    v[4] = v[2] * (1.0 + v[1] * (((float) v[3] / v[0]) - 1.0));
}
RESULT ( v[4] )


EQUATION("MAtotal"){
/*
 * Auxiliar da equacao MarketShare. Necessario para normalizar os valores de M entre 0 e 1
 * Calcula a soma de todos os M
 */
    v[0] = 0.0;
    CYCLE(cur, "Firm"){
        v[0] += VS(cur, "MA");
    }

}
RESULT(v[0])


EQUATION("M"){
/*
 * Market share normalzado
 */
    v[0] = V("MA");
    v[1] = V("MAtotal");
    
    v[2] = v[0] / v[1];
}
RESULT( v[2] )


EQUATION("Mk"){
/*
 * Calula o Markup das firmas
 */
    v[0] = VL("Mk", 1);                 // Markup tempo anterior
    v[1] = V("Mk_sensibility");         // Parametro 
    v[2] = V("Qd");                     // Quantidade demandada
    v[3] = V("Qs");                     // Quantidade ofertada
    
    v[4] = v[0] * (1.0 + v[1] * (v[2] - v[3]) / v[3]);
}
RESULT ( v[4] )


EQUATION("NumHedge"){
/*
Contador de sistemas tipo Hedge
Eq. 21
*/
    v[0] = 0.0;
    CYCLE(cur, "Firm")
    {
        v[1] = VS(cur, "Class2");
        
        if(v[1] == 1)
            v[0]++;
    }
}
RESULT( v[0] )


EQUATION("NumHedge_Pct"){
    v[0] = V("NumHedge");
    v[1] = V("Qtd_firmas");
    v[2] = v[0] / v[1];
}
RESULT ( v[2] )


EQUATION("NumSpec"){
/*
Contador de sistemas tipo Especulativo
Eq. 22
*/
    v[0] = 0.0;
    CYCLE(cur, "Firm")
    {
        v[1] = VS(cur, "Class2");
        
        if(v[1] == 2)
            v[0]++;
    }
}
RESULT( v[0] )


EQUATION("NumSpec_Pct"){
    v[0] = V("NumSpec");
    v[1] = V("Qtd_firmas");
    v[2] = v[0] / v[1];
}
RESULT ( v[2] )


EQUATION("NumPonzi"){
/*
Contador de sistemas tipo Ponzi
Eq. 23
*/
    v[0] = 0.0;
    CYCLE(cur, "Firm")
    {
        v[1] = VS(cur, "Class2");
        
        if(v[1] == 3)
            v[0]++;
    }
}
RESULT( v[0] )


EQUATION("NumPonzi_Pct"){
    v[0] = V("NumPonzi");
    v[1] = V("Qtd_firmas");
    v[2] = v[0] / v[1];
}
RESULT ( v[2] )


EQUATION("P"){
/*
 * Equacao do Preco
 * Referente a cada firma
 * Eq. 7 do artigo
*/
    v[1] = V("ADebitB");        // Dividas acumuladas brutas/iniciais
    v[2] = V("AProfitD");       // Lucro das firmas considerando pagamento de despesas
    
    if ((v[1] <= 0.0 && v[2] <= 0.0) ||  V("Qs") == 0)
        v[0] = VL("P", 1);
    else {
        v[3] = VL("Mk", 1);    // Valor laggado de Mk e r pois ainda nao foram calculados
        v[4] = VL("r", 1);     // Mk: markup, r: taxa de juros de mercado (seguindo o modelo do artigo, pois no NetLogo seria 'z')
        v[5] = V("Qs");        // Quantidade ofertada  

        v[0] = v[3] * ( v[1] * (1.0 + v[4]) + v[2] ) / v[5];
        
    }

}        
RESULT( v[0] )


EQUATION("Pbank"){
/*
 * Preço limite estabelecido pelo banco (critério para concessão de financiamento)
 * Equação em BankBehaviour
*/ 
    v[0] = V("Pmed");
    v[1] = V("g");

    if (v[1] == 0)
        v[0] = VL("Pbank", 1);
    else
        v[0] /= v[1];

}
RESULT(v[0])


EQUATION("Pmed"){
/*
 * Preco medio determinado considerando apenas firmas Hedge
 */
    v[0] = 0.0;
    CYCLE(cur, "Firm"){                 // loop para cada firma
        if ( VLS(cur, "Class2", 1) == 1){
            v[1] = VS(cur, "P");        // preco
            v[2] = VS(cur, "M");        // market share
            
            v[0] += ( v[1] * v[2] ); 
        }
    }

}
RESULT( v[0] )  


EQUATION("PmedGeral"){
/*
 * Preco medio determinado considerando todas as firmas 
 */
    v[0] = 0.0;
    CYCLE(cur, "Firm"){                 // loop para cada firma
        
        v[1] = VS(cur, "P");        // preco
        v[2] = VS(cur, "M");        // market share
        
        v[0] += ( v[1] * v[2] ); 

    }
 
}
RESULT( v[0] )  


EQUATION("Qd"){
/*
 * Quantidade demandada
 * No artigo eh representado apenas por D
 * Referente a Eq. 14-16 
 */
    
    v[0] = V("M");
    v[1] = V("Yd");
    v[2] = V("P");
    
    v[3] = (v[0] * v[1]) / v[2];    

}
RESULT(v[3])


EQUATION("Qe"){
    // Atribuicoes abaixo estao no CreditMarket do NetLogo
    // Referente a Eq. 2 do modelo
    v[0] = V("E");
    v[1] = VL("V", 1);   // Volume de estoque do tempo anterior
    v[0] -=  v[1];       // Desconta da demanda esperada (Qe) os estoques do tempo anterior
    
    if (v[0] < 0){       // Ajuste necessario para quando Qe ficar negativo 
        v[0] = 0.0;    
    }
}
RESULT ( v[0] )


EQUATION("Qs"){
/*
Quantidade total ofertada (producao)
Eq. 1
*/
    v[0] = V("T");	     // Coeficiente tecnico de tranformacao (p)
    v[1] = V("L");	     // Insumos (v)
    v[2] = V("F");	     // Custo Fixo (p)
    v[3] = VL("V", 1);   // Estoques
    
    v[3] += ( v[0] * (v[1] - v[2]) );
}
RESULT( v[3] )


EQUATION("Qtd_firmas"){
/*   
 *  Executa apenas a primeira vez 
 *  Conta a quantidade de firmas totais no modelo
 *  Criei para auxiliar no calculo do primeiro Market Share (M)
 */
    v[0] = COUNT("Firm");
    
}
RESULT(v[0])


EQUATION("r"){
/*
 * Calculo do Juros
*/ 
    v[0] = V("spread");             // spread bancario
    v[1] = V("i");                  // taxa basica de juros
    
    v[2] = (1 + v[0]) * v[1];

}
RESULT(v[2])


EQUATION("R"){
/*
* Receita obtida por cada firma
* Parte 1/3 das Eq. 14, Eq. 15 e Eq. 16
*/
    v[0] = V("Qs");									// Quantidade total ofertada (v)
    v[1] = V("Qd");									// Demanda efetiva - monetaria.
    v[2] = V("P");									// Preço (v)

    if (v[1] <= v[0])
        v[3] = v[2] * v[1];
    else
        v[3] = v[2] * v[0]; 
}
RESULT( v[3] )


EQUATION("red"){
    v[0] = V("KLDtotal");          // total da demanda de credito pelas firmas
    v[1] = V("KbankL");            // Capital do banco disponível para emprestimo
    
    v[2] = 1.0;
    if (v[0] > v[1])              // se demanda de crédito superar capital disponível (v[3]), cria redutor
        v[2] = v[1] / v[0];
}
RESULT( v[2] )


EQUATION("Rtot"){
/*
 * Recursos totais da firma (recursos totais + lucros acumulados)
 * Referente a Eq. 17
 */

    v[0] = V("R");                                 // receita de vendas
    v[1] = V("AProfitD");                          // Lucro das firmas considerando pagamento de despesas
    
    v[2] = v[0] + v[1];

}
RESULT(v[2])


EQUATION("S"){
/*
 * Renda nao gasta.
 * Somatorio dos valores individuais de Sfirm
 * Referente a Eqs. 14-16
 */

    v[0] = 0.0;
    
    
    CYCLE(cur, "Firm"){
        v[1] = VS(cur, "Qs");
        v[2] = VS(cur, "Qd");
        
        if (v[2] > v[1]) {
            v[3] = VS(cur, "P");
            v[0] += v[3] * (v[2] - v[1]);
        }
    }

}
RESULT(v[0])


EQUATION("Sd"){
/*
 * Saldo devedor nao honrado ou perda de capital do banco
 * Equacao em FinancingRegime do NetLogo
*/
    v[0] = V("SdL");            // Saldo devedor considerando o emprestimo pego
    v[1] = V("AM");             // Amortizacao
    
    v[2] = v[0] - v[1];         // creditar a amortizacao na dívida
    
    if (v[2] < 0) v[2] = 0.0;

}
RESULT(v[2])


EQUATION("SdL"){
/*
 * Saldo devedor considerando o emprestimo pego
 */
    v[0] = VL("Sd", 1);
    v[1] = V("KL");
    
    v[2] = v[0] + v[1];         // Saldo devedor não honrado ou perda de capital do banco do tempo anterior
}
RESULT( v[2] )


EQUATION("sigma"){
/*
 * Taxa desejada de aplicacao em titulos
*/ 
    v[0] = V("teto_Ktit");
    v[1] = V("piso_Ktit");
    v[2] = v[0] / v[1];             // g1 netlogo
    
    v[3] = V("spread");             // Spread - Frag2
    
    v[4] = v[1] * ( pow(v[2], v[3]) );
    
}
RESULT(v[4])


EQUATION("spread"){
/*
 * Spread bancario
 * Equivalente a variavel spread e Frag2 do NetLogo (eram a mesma coisa)
 * Serve como suavizacao de H - Frag1
 * Equacao em BankBehaviour
 * Equivalente a Eq. 32
*/ 

    v[0] = V("alpha");
    v[1] = VL("spread", 1);
    v[2] = V("Frag");
    
    v[3] = v[0] * v[1] + (1 - v[0]) * v[2];   // Frag2

}
RESULT(v[3])


EQUATION("time"){
/*
 * Contador de tempo.
 * Incrementa a cada passo da simulação 
*/ 
    v[0] = VL("time", 1);
    
}
RESULT(v[0] + 1)


EQUATION("Class2"){
/*
 * Status do para saber o tipo de regime financeiro que cada firma enfrenta em cada tempo
 * A contagem acontece considerando todos os agentes de cada tempo 
 * Eq. 21, Eq. 22, Eq. 23
 * Também auxiliara nas diferentes formas de calular as Eqs. 17, 18, 19 e 20 (segundo o NetLogo)
 * Os resultados serao salvos em Class2
*/

    v[0] = V("Rtot");              // Recursos totais da firma
    
    v[1] = VL("r", 1);             // taxa de juros de mercado
    v[2] = V("ADebitB");           // dividas acumuladas = (t2 NetLogo)
        
// Firmas do tipo Hedge = 1. Pagam toda dívida com o banco
    if( v[0] >= v[2]*(1+v[1]) )
        v[3] = 1;
// Firmas do tipo Speculativo = 2. Pagam apenas os juros de suas dívidas
    else{
        if(v[0] >= v[2]*v[1])
            v[3] = 2;
// Firmas do tipo Ponzi = 3. Não paga nada ao banco e saem do sistema
        else
            v[3] = 3;
    }
}
RESULT(v[3])


EQUATION("V"){
/*
 * Estoques 
 * Parte 3/3 das Eq. 14, Eq. 15 e Eq. 16
*/

    v[0] = V("Qs");									// Quantidade total ofertada (v) - tempo anterior
    v[1] = V("Qd");									// Demanda efetiva - monetaria.

    if (v[1] < v[0])
        v[2] = v[0] - v[1];
    else
        v[2] = 0.0; 
}
RESULT( v[2] )


EQUATION("Vtotal"){
/*
Total de estoques
RUN WORLD - set Vtotal
*/
    v[0] = SUML("V", 1);
}
RESULT( v[0] )


EQUATION("Wealth"){
/*
 * Saude do banco - riqueza total da economia
 * Equacao em BankBehaviour 
*/ 
    v[0] = V("Kbank");                  // Capital do banco    
    v[1] = V("AProfitTotal");           // Soma dos lucros (AProfit)
    v[2] = V("S");                      // Renda nao gasta
    
    v[3] = v[0] + v[1] + v[2];
    
}
RESULT(v[3])


EQUATION("Wl"){
/*
Despesas de produção dada a expectativa de demanda
NETLOGO: CreditMarke - nome original Wb
*/
    v[0] = V("W");	//salario ou preços dos insumos (p) - cosiderando valor fixo
    v[1] = V("LF");	//Insumos (v)
    v[2] = v[0] * v[1];

}
RESULT( v[2] )


EQUATION("Wtotal") {
/*
 * Capital monetário referente a todas as firmas
 * Variavel Wtotal do NetLOGO em production
 * Importante observar que esta um pouco diferente do modelo netlogo, pois la o Wtotal eh zerado
 *  Nao faremos isso aqui pois a cada passo de tempo as variveis sao zeradas
*/

    v[0] = SUM("K");
}    
RESULT ( v[0] )

EQUATION("Y"){
/*
 * Renda agregada.
 * Ainda chama a equacao de parcela de renda disponivel (aplicada em titulos) - Yg
 */

    v[0] = V("Wtotal");                     // Massa salarial
    v[1] = VL("S", 1);                      // Renda nao gasta (poupanca)
    
    v[0] +=  v[1];
}
RESULT ( v[0] )


EQUATION("Yd"){
/*
 * Renda agregada.
 * Ainda chama a equacao de parcela de renda disponivel (aplicada em titulos) - Yg
 */

    v[0] = V("Y");                     // Massa salarial
    v[1] = V("beta");                      // Renda nao gasta (poupanca)
    
    v[2] =  (1 - v[1]) * v[0];
}
RESULT ( v[2] )

EQUATION("beta"){
/*
 * Renda agregada.
 * Ainda chama a equacao de parcela de renda disponivel (aplicada em titulos) - Yg
 */

    v[0] = V("i");                     // Massa salarial
    v[1] = V("Ktit");                      // Renda nao gasta (poupanca)
    v[2] = V("Y");                         // Rendimento dos titulos
    
    v[3] =  v[0] * v[1] / v[2];
}
RESULT ( v[3] )

EQUATION("Yg"){
/*
 * Renda agregada.
 * Ainda chama a equacao de parcela de renda disponivel (aplicada em titulos) - Yg
 */

    v[0] = V("Y");                     // Massa salarial
    v[1] = V("beta");                      // Renda nao gasta (poupanca)
    
    v[2] =  v[1] * v[0];
}
RESULT ( v[2] )



EQUATION("Save_Information"){
    
    std::string expec = "";
    if V("ExpectativaHomogenea")
        expec = "_homo";
    else
        expec = "_hetero";
        
    std::string entry_ = "";
    if V("entry")
        entry_ = "_entry";
    else
        entry_ = "_no_entry";
    
    std::ofstream fout;
    
    std::string arq_name_sys = "resultados/sys_infos_time"+expec+entry_+"_"+data_+".csv";
    
    if (V("time") == 0) {
        fout.open(arq_name_sys);
        printf("%s\n", arq_name_sys.c_str());
    }
    else    
        fout.open(arq_name_sys, ios::app);
    
    // lista com todas as variaveis do sistema
    std::string vars[] = {"time", "Vtotal", "KLDtotal", "KtitM", "KbankL", "red", "Dc", "Ktit",
                          "Cred", "KLtotal", "KbankT", "Wtotal", "Kfail", "Cmed", "MAtotal", 
                          "Pmed", "Yg", "Y", "S", "JAMe", "JAMf", "Jtot", "h", "g", "Pbank", 
                          "Frag", "spread", "r", "sigma", "Kbank", "AProfitTotal", "Wealth", 
                          "Qtd_firmas", "NumHedge", "NumSpec", "NumPonzi", "PmedGeral"};
    
    // tamanho da lista criada acima
    int vars_size = *(&vars + 1) - vars;
    
    // Construção do cabeçalho do arquivo csv
    if (V("time") == 0){
        for (int i = 0; i < vars_size; i++)
            fout << vars[i] << ",";
        fout << "\n";
    }

    std::string var_name;
    // Contrução do conteúdo arquivo csv
    for (int i = 0; i < vars_size; i++){
        var_name = vars[i];
        fout << V( var_name.c_str() ) << ",";
    }
    fout << "\n";
    
    
    fout.close();
    
    std::ofstream fout2;
    std::string arq_name_firms = "resultados/firms_infos_time"+expec+entry_+"_"+data_+".csv";
    
    if (V("time") == 0) 
        fout2.open(arq_name_firms);
    else    
        fout2.open(arq_name_firms, ios::app);
    
    // lista com todas as variaveis da firma
    std::string  vars2[] = {"time", "id", "idTech", "A", "T", "F", "Qe", "LF", "Wl", "KLD", "KL",
                            "K", "AProfitD", "SdL", "ADebitB", "L", "Qs", "P", "C", "M", "Qd", 
                            "R", "V", "Mk", "Rtot", "AProfit", "J", "AM", "ADebit", "Sd", 
                            "Kmin", "Class1", "Class2"   
                            };
    
    // tamanho da lista criada acima
    vars_size = *(&vars2 + 1) - vars2;
    
    // Construção do cabeçalho do arquivo csv
    if (V("time") == 0){
        for (int i = 0; i < vars_size; i++)
            fout2 << vars2[i] << ",";
        fout2 << "\n";
    }

    // Contrução do conteúdo arquivo csv
    CYCLE(cur, "Firm"){
        for (int i = 0; i < vars_size; i++){
            var_name = vars2[i];
            fout2 << VS(cur, var_name.c_str() ) << ",";
        }
        fout2 << "\n";  
    }
    fout2.close();

}
RESULT(1)


EQUATION("menor_distancia_kmeans"){
    int qty_rows = 0;

    for (int i = 0; i < 300; i++){
        if (matriz_tech[i][4] == 1){
            qty_rows += 1;
        }
    }

    if (qty_rows < 3)
        v[5] = V("menor_distancia");
    else {
        double **matriz_hedge = new double*[qty_rows];
        for (int i = 0; i < qty_rows; i++) {
            matriz_hedge[i] = new double[3];
            for (int j = 0; j < 3; j++)
                matriz_hedge[i][j] = 0.0;
        }

        bool not_aadded_in_lastTech;
        double dists;
        int k = 0;
        int id = 0;


        v[5] = 0; //a
        v[6] = 0; //t
        v[7] = 0; //f

        CYCLE(cur, "Firm"){
            v[0] = VS(cur, "idTech");
            v[1] = VS(cur, "A");
            v[2] = VS(cur, "T");
            v[3] = VS(cur, "F");
            v[4] = VS(cur, "Class2");

            if (matriz_tech[ (int) v[0] ][4] == 1){
                for (int j = 0; j < 3; j++){
                    matriz_hedge[id][j] = v[j+1];
                }
                v[5] += v[1];
                v[6] += v[2];
                v[7] += v[3];
                id++;
            }
        }
        if (id > 0) {
            WRITE("a_media", v[5] / id);
            WRITE("t_media", v[6] / id);
            WRITE("f_media", v[7] / id);
            WRITE("qtd_real_hedge", id);
        }

        ClusterMethod cm;
        cm.GetClusterd(matriz_hedge, 3, qty_rows, 3);
        double **centers = cm.GetCentroids();
        int cluster_id = (int) uniform(0, 3);

        double menor_dist = HUGE_VAL;
        int id_menor_dist = -1;

        CYCLE(cur, "Tech"){
            v[0] = VS(cur, "At");
            v[1] = VS(cur, "Tt");
            v[2] = VS(cur, "Ft");
            v[3] = VS(cur, "idT");

            not_aadded_in_lastTech = true;
            for (auto it = lastTech.begin(); it != lastTech.end(); ++it){
                if (*it == v[3]){
                    not_aadded_in_lastTech = false;
                    break;
                }
            }

            if (not_aadded_in_lastTech){
                dists = 0.0;
                for (int j = 0; j < 3; j++)
                    dists += pow(v[j] - centers[cluster_id][j], 2);
                if (dists < menor_dist){
                    menor_dist = dists;
                    id_menor_dist = (int) v[3];
                }
            }
        }

        v[5] = id_menor_dist;

        lastTech.push_back( (int) v[5] );
        lastTech.pop_front();
        
        WRITE("a_c0", centers[0][0]);
        WRITE("t_c0", centers[0][1]);
        WRITE("f_c0", centers[0][2]);
        WRITE("a_c1", centers[1][0]);
        WRITE("t_c1", centers[1][1]);
        WRITE("f_c1", centers[1][2]);
        WRITE("a_c2", centers[2][0]);
        WRITE("t_c2", centers[2][1]);
        WRITE("f_c2", centers[2][2]);
        

        for(int i = 0; i < 3; i++){
            delete[] matriz_hedge[i];
            delete[] centers[i];
        }
        delete[] matriz_hedge;
        delete[] centers;
    }
}
RESULT(v[5])


EQUATION("menor_distancia"){

    double **centroids;
    centroids = new double*[3];
    // 1 linha para cada Class2
    for (int i = 0; i < 3; i++){
        centroids[i] = new double[4];
        // As 3 primeiras colunas são para os parametros A, T e F, respectivamente.
        // A 4a coluna sao referentes as quantidades 
        for (int j = 0; j < 4; j++)
            centroids[i][j] = 0.0;
        // centroids[i][3] = -1.0;
    }

    CYCLE(cur, "Firm"){
        v[0] = VS(cur, "idTech");
        v[1] = VS(cur, "A");
        v[2] = VS(cur, "T");
        v[3] = VS(cur, "F");
        v[4] = VS(cur, "Class2");

        if (matriz_tech[ (int) v[0] ][3] > 0){
            v[5] = matriz_tech[ (int) v[0] ][4] - 1;
            for (int j = 0; j < 3; j++)
                centroids[ (int) v[5] ][j] += v[j+1];
            centroids[ (int) v[5] ][3] += 1;
        }
    }

    for (int i = 0; i < 3; i++){
        if (centroids[i][3] > 0){
            for (int j = 0; j < 3; j++)
                centroids[i][j] /= centroids[i][3];
        }
    }

    double **distancias;
    distancias = new double*[3];
    // 1 linha para cada Class2
    for (int i = 0; i < 3; i++){
        distancias[i] = new double[2];
        // A primeira coluna eh para a menor distancia
        // A segunda coluna eh para o id da tecnologia
        for (int j = 0; j < 2; j++)
            distancias[i][j] = -1;
    }

    double dists;
    bool not_aadded_in_lastTech;
    int k = 0;
    CYCLE(cur, "Tech"){
        v[0] = VS(cur, "At");
        v[1] = VS(cur, "Tt");
        v[2] = VS(cur, "Ft");
        v[3] = VS(cur, "idT");

        not_aadded_in_lastTech = true;
        for (auto it = lastTech.begin(); it != lastTech.end(); ++it){
            if (*it == v[3]){
                not_aadded_in_lastTech = false;
                break;
            }
        }

        if (not_aadded_in_lastTech){
            for (int i = 0; i < 3; i++){
                dists = 0.0;
                if (centroids[i][3] > 0) {
                    for (int j = 0; j < 3; j++){
                        dists += pow(v[j] - centroids[i][j], 2);
                    }
                    if (dists < distancias[i][0] || distancias[i][0] == -1){
                            distancias[i][0] = dists;
                            distancias[i][1] = v[3];
                    }
                }
            }
        }
    }

    v[5] = -1;
    if (distancias[0][1] != -1)
        v[5] = distancias[0][1];

    lastTech.push_back( (int) v[5] );
    lastTech.pop_front();

    for(int i = 0; i < 3; i++){
        delete[] centroids[i];
        delete[] distancias[i];
    }
    delete[] centroids;
    delete[] distancias;
}
RESULT( (int) v[5] )


EQUATION("Incrementa_Matriz_Tech"){
    int principal_regime_val;
    int i;
    float div;

    // Incrementa a coluna referente ao atual Class2 de cada firma
    CYCLE(cur, "Firm"){
        i = (int) VS(cur, "idTech");
        v[1] = VS(cur, "Class2");


        v[2] = VL("r", 1);                 // taxa de juros de mercado
        v[3] = SUM("ADebitB");              // Total de debitos das firmas

        v[4] = VS(cur, "J");
        v[5] = VS(cur, "AM");

        v[6] = ((1 + v[2]) * v[3]) - (v[4] + v[5]);

        for (int j = 0; j < 3; j++)
            matriz_tech[i][j] *= dispersao;  

        div = matriz_tech[i][3];
        if (matriz_tech[i][3] == 0)
            div = v[6];

        matriz_tech[ i ][ (int) v[1] - 1] += v[6] / div;
        matriz_tech[i][3] = 0;

        principal_regime_val = HUGE_VAL;
        if V("bioinspirado") {
            matriz_tech[i][3] = 0;
            // evapora a matriz das tecnologias pelo valor da dispersao
            for (int j = 0; j < 3; j++){
                matriz_tech[i][3] += matriz_tech[i][j];
            }
        }
        else{
            matriz_tech[i][3] = 1;
        }
        for (int j = 0; j < 3; j++){
            if (matriz_tech[i][j] < principal_regime_val && matriz_tech[i][j] != 0){
                matriz_tech[i][4] = j+1;
                principal_regime_val = matriz_tech[i][j]; 
            }
        }
    }
}
RESULT( 1 )


MODELEND

// do not add Equations in this area

void close_sim( void )
{
	// close simulation special commands go here
}

