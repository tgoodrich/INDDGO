/*
  This file is part of INDDGO.

  Copyright (C) 2012, Oak Ridge National Laboratory 

  This product includes software produced by UT-Battelle, LLC under Contract No. 
  DE-AC05-00OR22725 with the Department of Energy. 

  This program is free software; you can redistribute it and/or modify
  it under the terms of the New BSD 3-clause software license (LICENSE). 
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  LICENSE for more details.

  For more information please contact the INDDGO developers at: 
  inddgo-info@googlegroups.com

*/

#include "GraphDecomposition.h"
#include "TreeDecomposition.h"
#include <fstream>
#include <unistd.h>

/*
 * This file generates one or more tree decompositions and 
 * calculates a variety of statistics (width, bag size distribution, 
 * average bag score), as well as optional treewidth lower bounds. 
 * It generates a single output file containing all requested 
 * information in a csv format. 
 * As usual, this operates on the largest connected component.
 */
int main(int argc, char **argv)
{
  /*current required arguments (in this order, no flags):
   * graph_file (dimacs)
   * kcore_file (scorefile format)
   * 
   */
  vector<double> *kcore_score = new vector<double>();
  vector<double> *degree_score = new vector<double>();
  int tdt[] = {TD_SUPERETREE, TD_GAVRIL, TD_BK, TD_NICE}; 
  int et[] = {GD_AMD, GD_METIS_NODE_ND, GD_METIS_MMD};
  vector<int> td_types(tdt, tdt+sizeof(tdt)/sizeof(tdt[0]));
  vector<int> elim_types(et, et+sizeof(et)/sizeof(et[0]));
  vector<double>* st[] = {kcore_score, degree_score};
  vector<vector<double> *> scores(st, st+sizeof(st)/sizeof(st[0]));

  /*necessary variables for storage*/
  char *kcore_file = argv[2]; 
  char *graph_file = argv[1];

  int t, e, i, s;
  Graph::WeightedMutableGraph *G;
  TDTree *T;
  double kcore_max, kcore_min; 
  int degree_max, degree_min;
     
  /*initialize log files if needed*/
  int pid;
#if WIN32 || _WIN32
  pid=_getpid();
#else
  pid=(int)getpid();
#endif
  char lfname[100];
  char efname[100];
  sprintf(lfname, "stats-%d.log",pid);
  sprintf(efname, "stats-%d.log",pid);
  //0: debug, 5: critical
  LOG_INIT(lfname, efname, 0);
  
  try
    {
      if(kcore_file == NULL || graph_file == NULL)
	throw(Graph::GraphException("Call with two arguments: graph_file kcore_file\n"));
      
      /*populate the graph*/
      Graph::create_largestcomponent_graph(graph_file, G);      
     
      /*populate appropriate score vectors*/
      bool range = read_color_file(kcore_file,kcore_max,kcore_min,*kcore_score);
      Graph::GraphUtil gutil; 
      gutil.recompute_degrees(G);
      vector<int> idegree_score = G->get_degree();
      (*degree_score).resize(idegree_score.size());
      for(int i = 0; i < idegree_score.size(); i++)
	(*degree_score)[i] = idegree_score[i];

      
      /*loop over tree decomposition algorithms*/
      for(t = 0; t < td_types.size(); t++)
	{
	  /*loop over elimination order heuristics*/
	  for(e = 0; e < elim_types.size(); e++)
	    {
	      /*form the tree decomposition*/
	      create_tree_decomposition(G, &T, false, NULL, false, 
					false, NULL, elim_types[e], 
					GD_UNDEFINED, td_types[t], 
					false, true);
	      //fill the bag vectors
	      T->fill_bag_vecs();
	      cout << "T has " << T->num_tree_nodes << " tree nodes\n";

	      //calculate the eccentricities
	      Graph::MutableGraph *GT = T->export_tree();
	      vector<int> ecc;
	      gutil.find_ecc(GT, &ecc);
	      cout << "Eccentricities " << ":\n"; 
	      for(int i = 0; i < ecc.size(); i++)
		cout << ecc[i] << " "; 
	      cout << "\n";
	      
	      vector<double> mystats;
	      cout << "Count " << ":\n"; 
	      bag_statistics(T, *(scores[0]), &mystats, GD_STAT_COUNT);
	      for(int i = 0; i < mystats.size(); i++)
		cout << mystats[i] << " "; 
	      cout << "\n";
	
	      /*loop over scores*/
	      for(s = 0; s < scores.size(); s++)
		{
		  cout << "Mean " << s << ":\n"; 
		  bag_statistics(T, *(scores[s]), &mystats, GD_STAT_MEAN);
		  for(int i = 0; i < mystats.size(); i++)
		    cout << mystats[i] << " "; 
		  cout << "\n";

		  cout << "Std Dev " << s << ":\n"; 
		  bag_statistics(T, *(scores[s]), &mystats, GD_STAT_STD);
		  for(int i = 0; i < mystats.size(); i++)
		    cout << mystats[i] << " "; 
		  cout << "\n";
		  
		}
	      
	      /*delete the tree decomposition*/
	      delete T;
	      
	    }
	}

      delete G;
      LOG_CLOSE();
      return 1;

    }
    catch(Graph::GraphException& e)
    {
      cerr << "exception caught: " << e.what() << endl;
      	LOG_CLOSE();
      return -1;
    }     
}


