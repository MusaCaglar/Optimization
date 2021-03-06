//this solves the ccsp of project portfolio selection with second order cone programming 
    #include <sstream>  // log file yazımı için
    #include <fstream>  //  log file yazımı için


    //#include "beta_distribution.hpp"
	#include "KPC_header.h"
	long int seed[15]={1363774876, 1511192140, 547070247 ,2096730329, 913566091, 1, 638580085, 
		1933576050, 246780520,  1259851944, 281629770, 1280689831,
	  824064364, 150493284, 242708531}; 

	int budget_fraction,prob_size,replication,p_strategy,Modified;
	float q,alfa1,beta1,alfa2,beta2,tau1,tau2,mean1,variance1,mean2,variance2; // (alfa1,beta1) 1. betanın parametreleri, (alfa2,beta2) 2. betanın parametreleri, 1. beta (0,tau1), 2. beta (tau2,1) arasında.
	float meanterm1,varianceterm1,meanterm2,varianceterm2;// in order to calculate parameters of beta fast meanterm=(mean-a)/(b-a), varianceterm=varaince/(b-a)^2
	

	FILE *fptr1,*fptr2,*fptr3,*fptr4; // for the first file fptr1 that stores results in a table format, second file fpr2 that stores generated data 
	char dataFilename[FILENAME_MAX]; // *fptr2 bunun icin. her problemi dosyaya atmak icin.. her seferinde yeni dosya ismi tanımlamak icin
	char dataFilename_sol[FILENAME_MAX]; // *fptr3 bunun icin. solutionları yazdırcaz CPLEX
	char dataLPfile[FILENAME_MAX]; // // *fptr4 bunun icin. Her solution LP file ını yazdırcaz

	int main(int argc, char **argv) {
   	
	//cplex environment
	IloEnv env;
	
	try {

	//generate random data
	//set the seed and solve problems

		//if Ri modified 1 o/w 0
		Modified=0; //



		//just give alfa beta tau values for ease of computation
		alfa1=0.95, beta1=1.25, alfa2=1.15, beta2=0.9, tau1=0.4, tau2=0.7;

for (prob_size=1000; prob_size <=2000;prob_size+=1000) {  //probsize 

  for (p_strategy=1;p_strategy <=2; p_strategy+=1) { // 1  0.01-0.1  2  0.01-0.2

    for (int q_strategy=1; q_strategy <=2; q_strategy+=1) { //1  q=0.4, 2  q=0.5

		if (q_strategy==1) q=0.4; else q=0.5; //

	  for (budget_fraction=1;budget_fraction<=2;budget_fraction++) {// to determine the available budget from total budget
		
			for(replication=0; replication <=4 ;replication++) {

				for(int confidence_strategy=3;confidence_strategy<=4;confidence_strategy++){

				fptr1 = fopen("results.txt","a") ;
	fprintf (fptr1,"%d0 \t %d \t %d \t %d \t %0.2f \t %0.2f \t %0.2f\t %0.2f \t %d\t",budget_fraction, prob_size, p_strategy, q_strategy,alfa1,beta1,alfa2,beta2,replication);
	//fprintf (fptr1,"\nCplex and DP times are:\n");
   	  fclose (fptr1);
	
	lcgrandst (seed[replication], replication);
	//printf ("%d\n", lcgrandst);

	// declare budgets and scores
	IloIntArray budget(env,prob_size+1);
	IloNumArray expected_budget(env,prob_size+1); //expected of budget
	IloNumArray variance_budget(env,prob_size+1); //variance of budget
	IloIntArray score(env,prob_size+1);
	IloNumArray prob(env,prob_size+1); //cancellation probability
	IloNumArray Expected_R(env,prob_size+1); //expected of truncated beta distribution Ri
	IloNumArray Variance_R(env,prob_size+1); //variance of truncated beta distribution Ri
	IloNumArray Expected_R2(env,prob_size+1); //expected of truncated beta distribution Ri square
	IloNumArray Expected_R3(env,prob_size+1); //expected of truncated beta distribution Ri cube
	//IloNumArray D_Kol_berry_essen;
	IloInt total_budget=0 ;// available budget icin
	IloInt available_budget=0;
	IloNum confidence_level=1.645; // 0 for %50, 0.53 for %70 0.84 for %80 döngüye mi atsak?? %90 z değeri 1.28

	if (confidence_strategy==0) { confidence_level=0.01; }		//%50
	else if (confidence_strategy==1) {confidence_level=0.53; }//%70
	else if (confidence_strategy==2) {confidence_level=0.84; }//%80
	else if (confidence_strategy==3) {confidence_level=1.28; }//%90
	else if (confidence_strategy==4) {confidence_level=1.64; }//95    


	//printf("generating begins---------------------------------------\n");
	for (int i=1; i<= prob_size;i++) {
			//budget[i]=(int)(5+25 * lcgrand(replication)); //budget in 10.000
			score[i]=(int)(10+15 * lcgrand(replication)); //score
	}

	/* approx. sectoral percent distribution in cumulative 10	20	35	45	60	85	100 */
     float s1=0.1,s2=0.2,s3=0.35,s4=0.45,s5=0.6,s6=0.85,s7=1;

	//like SEPS paper
	for (int i=1; i<=prob_size*s1;i++)							budget[i]=(int)(21+16 * lcgrand(replication)); //budget in 10.000
	for (int i=prob_size*s1+1; i<prob_size*s2;i++)				budget[i]=(int)(9+28 * lcgrand(replication));
	for (int i=prob_size*s2+1; i<=prob_size*s3;i++)				budget[i]=(int)(7+30 * lcgrand(replication));
	for (int i=prob_size*s3+1; i<=prob_size*s4;i++)				budget[i]=(int)(23+14 * lcgrand(replication));
	for (int i=prob_size*s4+1; i<=prob_size*s5;i++)				budget[i]=(int)(7+16 * lcgrand(replication));
	for (int i=prob_size*s5+1; i<=prob_size*s6;i++)				budget[i]=(int)(11+26 * lcgrand(replication));
	for (int i=prob_size*s6+1; i<= prob_size*s7;i++)			budget[i]=(int)(14+23 * lcgrand(replication));

	/* sectoral percent distribution 10	20	35	45	60	85	100 */
	

	for (int i=1; i<= prob_size;i++) {
	    if (p_strategy==1)  prob[i]=0.01+0.01*((int)(10*lcgrand(replication)));
		else prob[i]=0.01+0.01*((int)(20*lcgrand(replication)));
		//else prob[i]=0; //sabit pi=0;
		//printf("%f \n", prob[i]);
		total_budget+=budget[i];
	//write instances to files
	sprintf_s(dataFilename, sizeof(dataFilename),"PS%d_BF%d0_Pr%d_q%0.1f_a1_%0.2f_b1_%0.2f_a2_%0.2f_b2_%0.2f_cl_%0.2f_R%d.txt",prob_size,budget_fraction,p_strategy,q,alfa1,beta1,alfa2,beta2,confidence_level,replication);
   	  fopen_s (&fptr2,dataFilename,"a");  
	  fprintf (fptr2,"%d  \t %d \t %0.3f \n",budget[i], score[i], prob[i]);
	   fclose (fptr2); 
	}
	
	available_budget=(int)(total_budget*budget_fraction/10);
	
	// continue write instances to files
	fopen_s (&fptr2,dataFilename,"a");  
	  fprintf (fptr2,"BF: %d0 percent \t available budget: %d  \t q_strategy: %d \t p_strategy: %d \n",budget_fraction, available_budget, q_strategy, p_strategy);
	   fclose (fptr2); 
	
	//create decision variables
	IloNumVarArray x(env,prob_size+1,0,1,ILOINT); //binary var for selection
	IloNumVar      y(env, 0, IloInfinity,"y"); //y variable for conic quadratic constraint
	
	// SP model
	IloModel model(env);
	// form the objective function
	
	//maximize total score of selected projects
	IloExpr objective(env);		
	//printf("obj tanimlandi\n");
	for (int i = 1; i <= prob_size; i++) {
	//printf("dongu basladi\n");
	objective += score[i]*(1-prob[i]) * x[i];		         	
	//printf("olusturuyo %d\n",i);
	}
   	model.add(IloMaximize(env, objective));     
	objective.end();
	//printf("obj oldu\n");  


	IloExpr budget_expr1(env); //for expected term
	IloExpr budget_expr2(env); //for variance term
	
	//IloNum rounding=0;
	for(int i=1; i<=prob_size; i++){
		Expected_R[i]=prob[i]*((alfa1*tau1)/(alfa1+beta1))+q*((alfa2+beta2*tau2)/(alfa2+beta2))+(1-prob[i]-q);
		
		//son term modified Ri icin
		Variance_R[i]=prob[i]*(((alfa1*tau1)/(alfa1+beta1))*((alfa1*tau1)/(alfa1+beta1))+((tau1*tau1*alfa1*beta1)/((alfa1+beta1)*(alfa1+beta1)*(alfa1+beta1+1))))+
			q*(((alfa2+beta2*tau2)/(alfa2+beta2))*((alfa2+beta2*tau2)/(alfa2+beta2))+(((1*tau2)*(1-tau2)*alfa2*beta2)/((alfa2+beta2)*(alfa2+beta2)*(alfa2+beta2+1))))+
			(1-prob[i]-q)-(Expected_R[i]*Expected_R[i])-Modified*(prob[i]*variance1+q*variance2); //son term modified Ri icin
		
		expected_budget[i]=budget[i]*Expected_R[i];
		variance_budget[i]=budget[i]*budget[i]*Variance_R[i]; //rounded variance

		//for berry-essen bound calculation

		//modified Ri son minus term (prob[i]*variance1+q*variance2)
		Expected_R2[i]=prob[i]*(((alfa1*tau1)/(alfa1+beta1))*((alfa1*tau1)/(alfa1+beta1))+((tau1*tau1*alfa1*beta1)/((alfa1+beta1)*(alfa1+beta1)*(alfa1+beta1+1))))+
			q*(((alfa2+beta2*tau2)/(alfa2+beta2))*((alfa2+beta2*tau2)/(alfa2+beta2))+(((1*tau2)*(1-tau2)*alfa2*beta2)/((alfa2+beta2)*(alfa2+beta2)*(alfa2+beta2+1))))+
			(1-prob[i]-q) -Modified*(prob[i]*variance1+q*variance2) ;

		Expected_R3[i]=(1-Modified)*(prob[i]*((tau1*tau1*tau1)*(((alfa1+2)*(alfa1+1)*(alfa1))/((alfa1+beta1+2)*(alfa1+beta1+1)*(alfa1+beta1))))+(1-prob[i]-q)+
			q*(1-tau2)*(1-tau2)*(1-tau2)*(((alfa2+2)*(alfa2+1)*(alfa2))/((alfa2+beta2+2)*(alfa2+beta2+1)*(alfa2+beta2)))+
			3*q*(tau2)*(1-tau2)*(1-tau2)*(((alfa2+1)*(alfa2))/((alfa2+beta2+1)*(alfa2+beta2)))+
			3*q*(tau2)*(tau2)*(1-tau2)*(((alfa2))/((alfa2+beta2)))+
			q*(tau2)*(tau2)*(tau2))+Modified*((mean1*mean1*mean1)*prob[i]+(mean2*mean2*mean2)*q+(1-prob[i]-q));

	    //variance_budget[i]=0.01*((int)(100*((budget[i])*(budget[i])*prob[i]*(1-prob[i])+alfa*(budget[i])*(budget[i])*prob[i]*(alfa-2*(1-prob[i])-alfa*prob[i])))); //rounded variance
		//rounding=0.01*(int)(100*(1-prob[i])/confidence_level);
	budget_expr1 += expected_budget[i]*x[i]/confidence_level;  //expected budget of selected
    //budget_expr1 += (0.01*(int)(100*(expected_budget[i]/confidence_level)))*x[i];
	//budget_expr1 += budget[i]*x[i]*rounding;
	//rounding=0;
	}
	for(int i=1; i<=prob_size; i++){
		//rounding=0.01*((int)(100*(1-prob[i])*prob[i]));
	 budget_expr2 += variance_budget[i]*x[i]*x[i];   //variance term
	 //budget_expr2 += (budget[i])*(budget[i])*rounding*x[i]*x[i];   //variance term
	 //rounding=0;
	}
	//budget_expr=budget_expr1-budget_expr2;
	//rounding=0.01*((int)(100*(available_budget/confidence_level)));
	 //model.add(y==rounding-budget_expr1); // 1 kere ekle
	model.add(y==(available_budget/confidence_level)-budget_expr1); // 1 kere ekle
	 // model.add(y==(0.01*(int)(100*available_budget/confidence_level))-budget_expr1); // 1 kere ekle
	model.add(budget_expr2<=y*y); // 1 kere ekle
	budget_expr1.end();
	budget_expr2.end();
   

	//additional policy constraints

	IloExpr   area_x_1_expr(env); //for sum of xi in the area
	IloExpr   area_x_2_expr(env); //for sum of xi in the area
	IloExpr   area_x_3_expr(env); //for sum of xi in the area
	IloExpr   area_x_4_expr(env); //for sum of xi in the area
	IloExpr   area_x_5_expr(env); //for sum of xi in the area
	IloExpr   area_x_6_expr(env); //for sum of xi in the area
	IloExpr   area_x_7_expr(env); //for sum of xi in the area

	IloExpr    sum_x_i_expr(env); //for sum of all xi s

	for (int i=1; i<=prob_size*s1;i++)							area_x_1_expr+=x[i]; //budget in 10.000
	for (int i=prob_size*s1+1; i<=prob_size*s2;i++)				area_x_2_expr+=x[i];
	for (int i=prob_size*s2+1; i<=prob_size*s3;i++)				area_x_3_expr+=x[i];
	for (int i=prob_size*s3+1; i<=prob_size*s4;i++)				area_x_4_expr+=x[i];
	for (int i=prob_size*s4+1; i<=prob_size*s5;i++)				area_x_5_expr+=x[i];
	for (int i=prob_size*s5+1; i<=prob_size*s6;i++)				area_x_6_expr+=x[i];
	for (int i=prob_size*s6+1; i<= prob_size*s7;i++)			area_x_7_expr+=x[i];

	for (int i=1; i<= prob_size;i++)									sum_x_i_expr+=x[i];

	model.add(area_x_1_expr>=0.1*sum_x_i_expr);
	model.add(area_x_2_expr>=0.1*sum_x_i_expr);
	model.add(area_x_3_expr>=0.1*sum_x_i_expr);
	model.add(area_x_4_expr>=0.1*sum_x_i_expr);
	model.add(area_x_5_expr>=0.1*sum_x_i_expr);
	model.add(area_x_6_expr>=0.1*sum_x_i_expr);
	model.add(area_x_7_expr>=0.1*sum_x_i_expr);

	area_x_1_expr.end();
	area_x_2_expr.end();
	area_x_3_expr.end();
	area_x_4_expr.end();
	area_x_5_expr.end();
	area_x_6_expr.end();
	area_x_7_expr.end();

	//define the stopwatch
	IloTimer timer(env);
	IloNum cputime, cputime1;  
	
   //form the SP model in cplex
    IloCplex cplex(model);
	
	
	//tekrarlı log file yazımı
	std::stringstream s; 
	s << "PS" << prob_size <<"_BF"<< budget_fraction <<"0_Pr" << p_strategy << "_q" << q << "_a1"<<alfa1 << "_b1"<< beta1<< "_a2" << alfa2 << "_b2"<< beta2 <<"_cl" << confidence_level<< "_R"<< replication <<"_logfile.txt"; 
    std::ofstream logfile(s.str().c_str()); // use the stream here 
	cplex.setOut(logfile); //setOut fonksiyonu Log file ları yazdırır 

	// tekrarlı LP file 
	char model_LP_file[150];
        sprintf_s(model_LP_file,"PS%d_BF%d0_Pr%d_q_%0.1f_a1_%0.2f_b1_%0.2f_a2_%0.2f_b2_%0.2f_cl_%0.2f_R%d.lp",prob_size,budget_fraction,p_strategy,q,alfa1,beta1,alfa2,beta2,confidence_level,replication);
        cplex.exportModel(model_LP_file);

    // tekrarlı sav file: sav interactive optimizasyonla aynı sonucu veriyor. LP vermeyebiliyor. LP nin binary versiyonu sav.
	char model_sav_file[150];
        sprintf_s(model_sav_file,"PS%d_BF%d0_Pr%d_q_%0.1f_a1_%0.2f_b1_%0.2f_a2_%0.2f_b2_%0.2f_cl_%0.2f_R%d.sav",prob_size,budget_fraction,p_strategy,q,alfa1,beta1,alfa2,beta2,confidence_level,replication);
        cplex.exportModel(model_sav_file);

    //cplex.exportModel("KPC.sav"); // writing lp model to file
	cplex.setParam(IloCplex::ClockType, 1); // 1 CPU time, 2  wall time
	cplex.setParam(IloCplex::TiLim, 10800); //time limit 10800, i.e. 3hrs
	 //cplex.setParam(IloCplex::EpGap, 0.0001); //default relative gap
	//cplex.setParam(IloCplex::EpAGap, 0.000001); //default absolute gap
	//cplex.setParam(IloCplex::MIQCPStrat, 2); //only for unsolved instances, 1 quadratic relax, 2 lp relax
	//cplex.setParam(IloCplex::NumericalEmphasis, 1); // emphasize numerical precision
	//cplex.setParam(IloCplex::BarQCPEpComp, 0.0000001); // default 
	//cplex.setParam(IloCplex::BarQCPEpComp, 0.0001); // default
	
	//to invoke cuts for conic models you need to include the following parameter setting:
     cplex.setParam(IloCplex::Param::MIP::Cuts::LiftProj, 2);
	
	 //solve the problem
	timer.restart();
	cplex.solve();
    cputime = timer.stop();
	
	const char *status;
	if (cplex.getStatus() ==  IloAlgorithm::Optimal) 
		status= "Optimal";
	else status="Not Optimal";

	float sumbudgetcplex=0,sumbudgetvariance=0; // to find expected budget of each solution in cplex
	float sumcplex=0; //to find number of selected
	float sumexpectedcancellation=0,sumcancellationvariance=0, sumskewness=0; //for poisson binomial distribution
	float sumbudget_std_dev=0;
	float D_Kol_berry_essen=0;
	float D_Kol_berry_essen_term1=0; // her bir i ı sum a atma
	float D_Kol_berry_essen_term2=0; // her bir i ı sum a atma
	float D_Kol_berry_essen_lb=0;
	float D_Kol_berry_essen_ub=0;
	for(int i=1;i<=prob_size; i++){  // to find the budget of each solution in cplex
   sumcplex+=cplex.getValue(x[i]);
   sumbudgetcplex+=expected_budget[i]*cplex.getValue(x[i]);//sum of expected
   sumbudgetvariance+=variance_budget[i]*cplex.getValue(x[i]); //sum of variance term
   sumexpectedcancellation+=prob[i]*cplex.getValue(x[i]);//for poisson binomial distribution
   sumcancellationvariance+=prob[i]*(1-prob[i])*cplex.getValue(x[i]);//for poisson binomial distribution
   sumskewness+=prob[i]*(1-prob[i])*(1-2*prob[i])*cplex.getValue(x[i]);//for poisson binomial distribution
   
   D_Kol_berry_essen_term1+=abs(cplex.getValue(x[i])*(budget[i]*budget[i]*budget[i]*Expected_R3[i]
   -3*budget[i]*budget[i]*budget[i]*Expected_R2[i]*Expected_R[i]
   +2*budget[i]*budget[i]*budget[i]*Expected_R[i]*Expected_R[i]*Expected_R[i]));
   
   D_Kol_berry_essen_term2+=cplex.getValue(x[i])*(budget[i]*budget[i]*Variance_R[i]);
	
	}
 
	sumbudgetcplex+=confidence_level*sqrt(sumbudgetvariance);//sum of expected(yukarıda)+confidence_level*sqrt(variance)
	sumbudget_std_dev=sqrt(sumbudgetvariance);
	
	D_Kol_berry_essen=D_Kol_berry_essen_term1/(sqrt(D_Kol_berry_essen_term2*D_Kol_berry_essen_term2*D_Kol_berry_essen_term2));
	D_Kol_berry_essen_lb=D_Kol_berry_essen*0.4097;
	D_Kol_berry_essen_ub=D_Kol_berry_essen*0.56;
	//CplexStatus CplexObjective	CplexCPUtime CplexBestUB CplexBestLB 
	 IloNum MIP_gap=0; //MIP gap i hesaplamak için
	 MIP_gap=100*(cplex.getBestObjValue()-cplex.getObjValue()) /cplex.getObjValue();
	 fptr1 = fopen("results.txt","a") ;
     fprintf(fptr1," %d \t %s \t %0.2f \t %0.2f \t %0.2f \t  %0.2f \t %0.0f \t %0.2f \t %0.2f \t %0.2f \t %0.4f \t %0.4f \t %0.4f \t %0.4f\t %0.4f \t %0.4f\n",available_budget,status,cplex.getObjValue(),cplex.getBestObjValue(),MIP_gap,cputime,sumcplex,sumbudgetcplex,sumbudgetvariance,sumbudget_std_dev,D_Kol_berry_essen_lb,D_Kol_berry_essen_ub,sumexpectedcancellation,sumcancellationvariance,sumskewness,confidence_level);
	 fclose (fptr1); 

	printf("%0.2f  %s\n",cplex.getObjValue(),status);
	
	/*
	for(int i=1;i<=prob_size;i++){
	sprintf_s(dataFilename_sol, sizeof(dataFilename_sol),"Stochastic_PS%d_BF%d0_Prob%d_q_%0.1f_alfa1_%0.2f_beta1_%0.2f_mean1_%0.2f_variance1_%0.2f_alfa2_%0.2f_beta2_%0.2f_mean2_%0.2f_variance2_%0.2f_R%d_CPLEX_solution.txt",prob_size,budget_fraction, p_strategy,q,alfa1,beta1,mean1,variance1,alfa2,beta2,mean2,variance2, replication);
   	  fopen_s (&fptr3,dataFilename_sol,"a");  
	  fprintf (fptr3,"%d %0.1f \t %0.3f \t %d\n",i,cplex.getValue(x[i]), prob[i],budget[i]);
	   fclose (fptr3); 
	}   */

		
	for(int i=1;i<=prob_size;i++){
	sprintf_s(dataFilename_sol, sizeof(dataFilename_sol),"PS%d_BF%d0_Prob%d_q%0.1f_a1_%0.2f_b1_%0.2f_a2_%0.2f_b2_%0.2f_cl_%0.2f_R%d_soln.txt",prob_size,budget_fraction, p_strategy,q,alfa1,beta1,alfa2,beta2,confidence_level,replication);
   	  fopen_s (&fptr3,dataFilename_sol,"a");  
	  fprintf (fptr3,"%d \t %0.1f \t %0.3f \t %d\n",i,cplex.getValue(x[i]), prob[i],budget[i]);
     //simulation
	 // if (lcgrand(replication*2)<=prob[i]) {string status="canceled"};

	   fclose (fptr3); 
	}   
	cplex.clear();

    cplex.end(); 
	
   }//end of for loop confidence level

	}//end of for loop replication

	} //end of for loop budget_fraction
	} //end of for q_strategy
	} //end of for p_strategy 
	} // end of for loop prob_size

	//close the environment
		env.end();	
      
	} //end of try

	catch (IloException& e) {
	cerr << e << endl;
	}  //end of catch
	//close the environment
	env.end(); // free all memory
	return 0;
	} //end of main function
