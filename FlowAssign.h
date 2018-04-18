#ifndef FLOWASSIGN
#define FLOWASSIGN
#include <list>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>

typedef long long int size_of_cost;

#include "HopcroftKarp.h"

#define VERTEX 0x3FFFFFFF
#define SATURATED 0x40000000

class FlowAssign{
private:
	bool *is_a_deficit;
	int C_up;
	int number_women, number_men;
	int *virtual_men;
	int *virtual_source;
	int *married;
	//this vector holds the relative position of the neighbor/edge of a woman who points to a men by a saturated arc
	int *indices_of_current_saturated_arcs;
	long long int *pd;
	void DetermineValueC_up(Vertex*& f_NG){
		C_up = 0;
		for(int v = this->lower_women; v <= this->upper_women; v++)
			for(int j = 0; j < f_NG[v].length; j++)
				C_up = f_NG[v].cost[j] < 0 ? std::max(C_up, -f_NG[v].cost[j]) : std::max(C_up, f_NG[v].cost[j]);
		return;
	}
public:
	int number_vertices, lower_men, lower_women, upper_men, upper_women, source, sink;
	FlowAssign(){
		C_up = 0;
		return;
	}
/*
	Created By: Sergio Perez
	Modified by: 
	Revised By: 
	Created on: March, 2013
	Last Update on: Dec, 2014
	This function allows to handle all the possible errors which may occur during the program
*/
void Message_error(int error){
	std::string dic[34];
	dic[10] = "Some indices of the vertices are out of range.";
	dic[11] = "The expected input values can not be retrieved from the input.";
	dic[12] = "The EOF was reached before of the expected.";
	dic[13] = "The size of the printed matching does not match with the specified matching size.";
	dic[14] = "The Lambda bound was overcome.";
	dic[15] = "The number of arcs visited in build the shortest-path forest was bigger than the number of arcs.";
	dic[16] = "The size of the flow obtained from the match is different to the size of the match in M.";
	dic[17] = "Some saturated arc is not proper.";
	dic[18] = "Some idle arc is not proper.";
	dic[19] = "Something fails in the method to detect the bad choice for k when the final prices are raised.";
	dic[20] = "Some saturated arc is not epsilon-proper.";
	dic[21] = "Some saturated arc is not epsilon-snug.";
	dic[22] = "Some idle arc is not epsilon-snug.";
	dic[23] = "The status of this edge should be idle, but it is saturated.";
	dic[24] = "The status of this edge should be saturated, but it is idle.";
	dic[25] = "Some reduced cost cp at the end of the algorithm, for a saturated arc, is wrong.";
	dic[26] = "The input type indicated does not exist. Please verify your parameters.";
	dic[27] = "The given input can not be generated in output text format.";
	dic[28] = "The vertex was already processed by the loop in the procedure Build the forest.";
	dic[29] = "The specified input file does not exist.";
	dic[30] = "The specified output file does not exist.";
	dic[31] = "The index was out of range.";
	dic[32] = "A negative lentgh for a link was reached.";
	dic[33] = "Some neighbors were calculated in a wrong way.";
	if(10 <= error && error <= 33)
		std::cout << "ERROR " << error <<": " << dic[error] << "\n";
	else
		std::cout << "An error occurred.\n";
	exit(error);
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function returns lp value of a saturated arc which will be used to build the forest
	Input:
		cp: size_of_cost; the value of the reduced cost (c(v, w)-pd(v)+pd(w))
*/
inline size_of_cost Lp_backward(size_of_cost cp, const int positions_current_epsilon){
	return 1-(-((-cp)>>positions_current_epsilon));
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function returns lp value of an idle arc which will be used to build the forest
	Input:
		cp: size_of_cost; the value of the reduced cost (c(v, w)-pd(v)+pd(w))
*/
inline size_of_cost Lp_forward(size_of_cost cp, const int positions_current_epsilon){
	return -((-cp)>>positions_current_epsilon);
}

/*
	Created By: Sergio Perez
	Modified by:
	Revised By:
	Created on: March, 2013
	Last Update on: Dec, 2014
	This function sets the values for the allowed indices in the generic arrays (for women, men, source and sink );
	is better does not use the position 0 for some woman or man since 0 is considered as special value in some cases
*/
void Set_indices(const int number_women, const int number_men){
	number_vertices = number_women+number_men+1+1;// we must consider at the begining the index and the source	
	lower_women = 0;
	upper_women = number_women-1;
	lower_men = number_women;
	upper_men = number_women+number_men-1;
	source = number_women+number_men;//the index for the vertex source
	sink = number_women+number_men+1;//the index for the vertex sink
	return;
}


/*
	Created By: Sergio Perez
	Modified by:
	Revised By:
	Created on: April, 2013
	Last Update on:
	This function adds and arc to the structure bipartite graph structure f_NG
	Input:
		x: int; the vertex reference to agregate its neighbor
		y: int; the vertex who will be the neighbor of x
		cost: size_of_cost; the original cost of the edge
		status: int; the status of the arc in the bi-graph, by default is IDLE
*/
void Add_arc(const int x, const int y, const int cost, Vertex*& f_NG, const int status = 0){
	f_NG[x].neighbor[f_NG[x].length] = status|y;
	f_NG[x].cost[f_NG[x].length] = cost;
	f_NG[x].length++;//all arcs are IDLE and have cp = cost by default	
	return;
}


/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: April, 2013
	Last Update on: January, 2015
	Complexity: O(1) 
	This function inverts an arc (v->w) on the augmenting path
	Input:
		v: int; the origin vertex of the arc to invert
		w: int; the destiny vertex of the arc to invert
		index: int; the index of the arc in f_NG to update
*/
void Invert_arc(const int v, const int w, const int index, Vertex*& f_NG){
	if(this->lower_women <= v && v <= this->upper_women){//it is conected to a man
		if(w == this->source){
			if(!(virtual_source[v] & SATURATED))
				Message_error(24);
			virtual_source[v] &= VERTEX;
			indices_of_current_saturated_arcs[v] = INT_MAX;
			married[v] = this->source;//we should not point to a woman
		}
		else{//then w is a man
			if(f_NG[v].neighbor[index]&SATURATED)
				Message_error(23);
			f_NG[v].neighbor[index] |= SATURATED;
			indices_of_current_saturated_arcs[v] = index;
			married[v] = w, married[w] = v;
		}
	}
	else if(this->lower_men <= v && v <= this->upper_men){
		if(w == this->sink){
			if(virtual_men[v] & SATURATED)
				Message_error(23);
			virtual_men[v] |= SATURATED;
		}
		else{//then w is a woman
			if(!(f_NG[w].neighbor[index]&SATURATED))
				Message_error(24);
			f_NG[w].neighbor[index] &= VERTEX;
		}
	}
	else if(v == this->source){
		if(virtual_source[w] & SATURATED)
			Message_error(23);
		virtual_source[w] |= SATURATED;
	}
	else if(v == this->sink){
		if(!(virtual_men[w] & SATURATED))
			Message_error(24);
		virtual_men[w] &= VERTEX;
		married[w] = this->source;		
	}
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(m)
	This recursive function augments K in the pseudoflow along the path indicated from the zero links in the Residual 
	digraph. This function is equivalent to the function Finding_augmenting_paths
	The vector 'indices_for_saturated_arcs' indicates the relative position for the arc in the set 
	of neighbors of the vertex v that becomes to saturated while the augmentation is done.
	The vector wife_of will allow to know who is the wife of a man y or will denote that the man does not 
	have wife if the arcs becomes from saturated to idle.
	The main if in the loop is asking if we have a forward arc or a backward arc. Since the order of the 
	vertices is source->Women->Men->sink is possible to know in constant time if an arc is forward or backward
*/
bool DFS_augmenting_paths_on_line(const int v, int &count_index_source, int &count_index_sink, 
	const int positions_current_epsilon, const int positions_epsilon_den_down, bool*& marked, Vertex*& f_NG){
//	cout << v << "->" ;
	if(v != this->source && v != this->sink)
		marked[v] = false;
	if(is_a_deficit[v]){
		is_a_deficit[v] = false;
		return true;
	}
	if(v == this->source){
		for(; count_index_source <= this->upper_women; ){
			int w = count_index_source++;
			if(marked[w] && !(virtual_source[w]&SATURATED) && Lp_forward(pd[w] - pd[this->source], positions_current_epsilon) == 0){
				if(DFS_augmenting_paths_on_line(w, count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
					marked, f_NG)){
						Invert_arc(this->source, w, w, f_NG);
						return true;
				}
			}
		}	
	}
	else if(this->lower_men <= v && v <= this->upper_men){
		if(marked[this->sink] && !(virtual_men[v]&SATURATED) && Lp_forward(pd[this->sink] - pd[v], positions_current_epsilon) == 0){
			if(DFS_augmenting_paths_on_line(this->sink, count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
				marked, f_NG)){
					Invert_arc(v, this->sink, 0, f_NG);
					return true;
			}
		}
	}
	else if(v != this->sink){
		for(int i = 0; i < f_NG[v].length; i++){
			int w = f_NG[v].neighbor[i]&VERTEX;
			if(marked[w] && !(f_NG[v].neighbor[i]&SATURATED)){//si es idle
				if(Lp_forward((((size_of_cost)f_NG[v].cost[i]) << positions_epsilon_den_down) - pd[v] + pd[w], positions_current_epsilon) == 0){
					if(DFS_augmenting_paths_on_line(w, count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
						marked, f_NG)){
						Invert_arc(v, w, i, f_NG);
						return true;
					}
				}
			}
		}
	}
	if(this->lower_women <= v && v <= this->upper_women){// see the case 2
		if(virtual_source[v]&SATURATED && Lp_backward(pd[v] - pd[this->source], positions_current_epsilon) == 0){
			if(DFS_augmenting_paths_on_line(this->source, count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
				marked, f_NG)){
				Invert_arc(v, this->source, v, f_NG);
				return true;
			}
		}
	}
	else if(this->lower_men <= v && v <= this->upper_men){// see the case 3
		//we need to ask if the man v is pointing to a woman, and if answer is yes she is in fact his wife
		int &w = married[v];
		if(this->lower_women <= w && w <= this->upper_women && marked[w]){//can I visit the vertex?
			int i = indices_of_current_saturated_arcs[w];
			if(!(f_NG[w].neighbor[i]&SATURATED))
				Message_error(24);
			if((f_NG[w].neighbor[i]&VERTEX) != v)
				Message_error(33);//*/				
			if(Lp_backward((((size_of_cost)f_NG[w].cost[i]) << positions_epsilon_den_down) - pd[w] + pd[v], positions_current_epsilon) == 0){
				if(DFS_augmenting_paths_on_line(w, count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
					marked, f_NG)){
					Invert_arc(v, w, i, f_NG);
					return true;
				}
			}
		}
	}
	else if(v == this->sink){// see the case 4
		for(; count_index_sink <= this->upper_men; ){
			int w = count_index_sink++;
			if(marked[w] && (virtual_men[w] & SATURATED) && Lp_backward(pd[v]-pd[w], positions_current_epsilon) == 0){
				if(DFS_augmenting_paths_on_line(w, count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
					marked, f_NG)){
					Invert_arc(v, w, 0, f_NG);
					return true;
				}
			}
		}
	}
	return false;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	Complexity: O(|S|) plus O(m) which is the time spent for all the possible augmentations
	This function finds the augmenting paths by using a recursive function to do the work
*/
int Finding_augmenting_paths_recursive_on_line(std::vector<int> &S, const int positions_current_epsilon, const int positions_epsilon_den_down,
	Vertex*& f_NG){
//	cout << "Finding recursively and on line the augmenting paths and augmenting along them." << endl;
	int size_of_P = 0;
	std::vector<int> Surp = S;
	S.clear();
	bool *marked = new bool[this->number_vertices];
	for(int v = 0; v < this->number_vertices; v++)
		marked[v] = true;
	int count_index_source = this->lower_women, count_index_sink = this->lower_men;
	for(unsigned int i = 0; i < Surp.size(); i++){
		if(DFS_augmenting_paths_on_line(Surp[i], count_index_source, count_index_sink, positions_current_epsilon, positions_epsilon_den_down, 
			marked, f_NG))
			size_of_P++;
		else
			S.push_back(Surp[i]);
	}
	delete[] marked;
	return size_of_P;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February 2015
	Complexity: O(Lambda) 
	This function cleans each position of those pointing for the doubly-linked list Q until the value	
	indicated by the global variable Lambda since it indicates the position until we need
*/
void Make_heap(const int Lambda, std::list<int>*& Q){
	for(int i = 0; i <= Lambda; i++)
		Q[i].clear();
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(1) 
	This function inserts a vertex of cost = 'value' in the heap denoted by Q
	Input:
		vertex: int; the vertex to insert in the segment Q[value]
		value: size_of_cost; the index in the array Q to insert the element denoted by vertex
*/
inline void Insert(const int vertex, const size_of_cost value, std::list<int>*& Q, std::list<int>::iterator*& pointers){
	if(value < 0){
		std::cout << vertex << " " << value << "\n";
		Message_error(32);
	}
	Q[value].push_front(vertex);
	pointers[vertex] = Q[value].begin();
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(Lambda) 
	This function extracts and deletes a vertex with the minimum current cost
	Since B never decreases we need to start from B and look for an non-empty 
	array in Q in order to extract the vertex of minimum cost in the heap.
*/
inline int Delete_min(int &Betha, const int Lambda, std::list<int>*& Q){
//	std::cout << "Deleting the minimum element from B = "<<Betha << "\n";
	for(; Q[Betha].empty() && Betha <= Lambda; Betha++);
//	std::cout << "Deleting the minimum element from sal = "<<Betha << "\n";
	if(Betha > Lambda)
		Message_error(14);

	int v = *(Q[Betha].begin());
	Q[Betha].erase(Q[Betha].begin());
	return v;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(1) 
	This function decreases the value in the heap of the vertex w, from L_old to L.
	If we update l[w] after to decrease it, when this function is called we have 
	l[w] = L_old, and L only is used to put the vertex in the correct position. 
	This allow us to know where is the position we need to erase in the heap where is the vertex w.
	Input:
		w: int; The vertex to whom we want to decrease the cost.
		L: size_of_cost: The new cost of the vertex w.
*/
inline void Decrease_key(const int w, const size_of_cost L, size_of_cost*& l, std::list<int>*& Q, std::list<int>::iterator*& pointers){
	Q[l[w]].erase(pointers[w]);
	Insert(w, L, Q, pointers);
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(|S|) 
	This function sets the lengths of the surpluses in the array l to the value indicated by 'value'
	Input:
		value: long_cost; the value to assign in all the surpluses of the array l
*/
void Initializes_surpluses(std::vector<int> &S, size_of_cost*& l, std::list<int>*& Q, std::list<int>::iterator*& pointers){
//	std::cout << "For all surpluses sigma, set l(sigma) = 0 and Insert(sigma, 0).\n";
	for(unsigned int i = 0; i < S.size(); i++){
		l[S[i]] = 0;
		Insert(S[i], 0, Q, pointers);
	}
	return;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: April, 2013
	Last Update on:
	Complexity: O(1) 
	This function sets the lengths of the surpluses in the array l to the value indicated by 'value'
	Input:
		value: long_cost; the value to assign in all the surpluses of the array l
*/
void Update_the_heap(const int w, const size_of_cost L, const int Lambda, const int Lambda_big, 
	size_of_cost*& l, std::list<int>*& Q, std::list<int>::iterator*& pointers){
	if(L <= Lambda && L < l[w]){
		if(l[w] == Lambda_big)
			Insert(w, L, Q, pointers);
		else
			Decrease_key(w, L, l, Q, pointers);
		l[w] = L;
	}
	return;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	Complexity: O(number_vertices) plus the time spent in the procedure Find_links_of_zero_length_and_update_cp
	This function raises the prices after the forest has build.
*/
void Raise_prices_for_the_forest_nodes(const long long int positions_current_epsilon, const long long int* l, std::vector<int> &forest){
	int &delta = forest[forest.size()-1];
//	cout << "Raising the prices for the nodes in the forest. The delta vertex is "<<delta << endl;
	for(unsigned int i = 0; i < forest.size(); i++)
		pd[forest[i]] += (l[delta] - l[forest[i]]) << positions_current_epsilon;
	return;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on: April, 2013
	Complexity: O(m) where m is the number of edges in the pseudoflow.
	This function builds shortest-path forest in Refine.
	The forest is build until reach a deficit.
	First we analyze the IDLE arcs in order to retrieve the forward arcs of the Residual di-graph from the pseudoflow
	Second we need to look the possible arcs who are pointing to the vertex v through a saturated arc, since it will 
	be a backward arc in the residual di-graph, by keeping in mind that:
	1) The source vertex does not have any vertex pointing him through a saturated arc.
	2) For a woman the only posibility is that the source vertex is pointing to her through a saturated arc, so in 
	the worst case we have to analyze only that possible saturated arc, which will be a backward arc.
	3) For a woman the only posibility is to have one woman pointing to him through a saturated arc, so in the worst 
	case we have to analyze only one bipartite arc, and we can to know that information by asking if that man has 
	a wife, in fact only if that man has a wife, this will give us a saturated arc, but not otherwise.
	4) For the sink we need to look which men are pointing to him through a saturated arc, since they will give the
	backward arcs in the Residual digraph
*/
void Build_forest(const int s, const int h, const size_of_cost q, const int positions_current_epsilon, const int positions_epsilon_den_down,
	std::vector<int> &S, Vertex*& f_NG){
//	cout << "====================================Building the forest." << endl;
	//This data structure helps to build the forest and always need the same space
	bool *marked = new bool[this->number_vertices];

	int Lambda = std::min((int)(((q<<2)+4) * s / h), (int)INT_MAX);
	std::list<int> *Q = new std::list<int>[Lambda+1];
	std::list<int>::iterator *pointers = new std::list<int>::iterator[number_vertices];
	size_of_cost *l = new size_of_cost[number_vertices];

	Make_heap(Lambda, Q);
	int Betha = 0, Lambda_big = std::min(Lambda+1, INT_MAX);
	std::vector<int> forest;

	for(int v = 0; v < number_vertices; v++) marked[v] = false, l[v] = Lambda_big;

	Initializes_surpluses(S, l, Q, pointers);

	int v;
	do{
		v = Delete_min(Betha, Lambda, Q);
		if(marked[v])
			Message_error(28);
		marked[v] = true;//this is only to validate this loop
//		cout << "The current v = " << v << " with "<<f_NG[v].size() << " neighbors and l(v) = "<<l[v]<<".\n";		
		//for all links v => w leaving v in Rf
		//first we are looking for the forward arcs from the IDLE arcs in the pseudoflow
		if(v == this->source){
			for(int w = this->lower_women; w <= this->upper_women; w++){
				if(!(virtual_source[w]&SATURATED))
					Update_the_heap(virtual_source[w]&VERTEX, l[this->source] +
					Lp_forward(pd[virtual_source[w]&VERTEX] - pd[this->source], positions_current_epsilon), Lambda, Lambda_big, l, Q, pointers);
			}
		}
		else if(this->lower_men <= v && v <= this->upper_men){
			if(!(virtual_men[v] & SATURATED))
				Update_the_heap(virtual_men[v]&VERTEX, l[v] + Lp_forward(pd[virtual_men[v] & VERTEX] - pd[v], positions_current_epsilon), 
						Lambda, Lambda_big, l, Q, pointers);
		}
		else if(v != this->sink){//then v is women
			for(int i = 0; i < f_NG[v].length; i++)
				if(!(f_NG[v].neighbor[i]&SATURATED))
					Update_the_heap(f_NG[v].neighbor[i]&VERTEX, l[v] +
						Lp_forward((((size_of_cost)f_NG[v].cost[i]) << positions_epsilon_den_down) - pd[v] + pd[f_NG[v].neighbor[i]&VERTEX], positions_current_epsilon), 
						Lambda, Lambda_big, l, Q, pointers);
		}
		//after that we are looking for the backward arcs from the saturated arcs pointing to the vertex v
		if(this->lower_women <= v && v <= this->upper_women){// see the case 2
			if(virtual_source[v]&SATURATED)
				Update_the_heap(this->source, l[v] + Lp_backward(pd[v] - pd[this->source], positions_current_epsilon), Lambda, Lambda_big, l, Q, pointers);
		}
		else if(this->lower_men <= v && v <= this->upper_men){// see the case 3
			//we need to ask if the man v is pointing to a woman, and if answer is yes she is in fact his wife
			int &w = married[v];
			if(this->lower_women <= w && w <= this->upper_women){
				int &i = indices_of_current_saturated_arcs[w];
				if(!(f_NG[w].neighbor[i]&SATURATED))
					Message_error(24);
				if((f_NG[w].neighbor[i]&VERTEX) != v)
					Message_error(33);
				Update_the_heap(w, l[v] + Lp_backward((((size_of_cost)f_NG[w].cost[i]) << positions_epsilon_den_down) - pd[w] + pd[v], positions_current_epsilon), 
					Lambda, Lambda_big, l, Q, pointers);
			}
		}
		else if(v == this->sink){// see the case 4
			for(int w = this->lower_men; w <= this->upper_men; w++)
				if(virtual_men[w] & SATURATED)
					Update_the_heap(w, l[v] + Lp_backward(pd[this->sink] - pd[w], positions_current_epsilon), Lambda, Lambda_big, l, Q, pointers);
		}

		forest.push_back(v);//Add v to the forest;
	}while(!is_a_deficit[v]);
	Raise_prices_for_the_forest_nodes(positions_current_epsilon, l, forest);

	for(int i = 0; i <= Lambda; i++)
		Q[i].clear();
	delete[] Q;
	delete[] pointers;
	delete[] marked;	
	delete[] l;
	forest.clear();
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(|X|)
	This function gets the s women matched by considering the status of the arcs from the source to each woman
	and by filling the vector is_a_surplus we are able to know in constant time if a vertex is a surplus.
	The vector is_a_surplus only is used in the function for raise the prices as in 7.4, that's why it does not 
	require to be updated when the augmentations are done.
*/
void Set_S(std::vector<int> &S){
	S.clear();//cleaning the current surpluses vector
	for(int v = this->lower_women; v <= this->upper_women; v++){
		if(virtual_source[v] & SATURATED){
			S.push_back(v);
			married[v] = this->source;
		}
	}
	return;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on: April, 2013
	Complexity: O(|Y|)
	This function gets the deficits men by considering the status from each men to the sink, 
	since only saturated arcs between a man and the sink denotes that the man is a deficit.
	The vector wife_of denotes who is the wife of a man (by a saturated arc) in the psudo_flow f_NG 
	or returns the source vertex otherwise, and it will be updated when the augmentations are done, 
	this vector will be used in the procedure to build the forest.
*/
void Set_D(){
	for(int y = this->lower_men; y <= this->upper_men; y++){
		married[y] = this->source;//allow to know if the saturated arc is pointing to a woman
		is_a_deficit[y] = (virtual_men[y] & SATURATED) ? true : false;
	}
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(|S|) where |S| = |f|
	This function converts the s bipartite arcs that are saturated in f to idle;
	We just need to look into the surpluses in order to retrieve such a bipartite saturated arcs
*/
void Convert_saturated_bipartite_arcs_to_idle_in_F(std::vector<int> &S, Vertex*& f_NG){
//	std::cout << "Converting the s bipartite arcs that are saturated in f to idle.\n";
	for(unsigned int i = 0; i < S.size(); i++){
		int &x = S[i];
		//if the edge from x -denoting a saturated arc to a man- is IDLE, then we have an error because such edge should be SATURATED.
		if(!(f_NG[x].neighbor[indices_of_current_saturated_arcs[x]]&SATURATED))
			Message_error(24);
		f_NG[x].neighbor[indices_of_current_saturated_arcs[x]] &= VERTEX;
	}
//	std::cout << "End Converting the s bipartite arcs that are saturated in f to idle.\n";
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity for raise the prices: O(|X|+|Y|)
	Complexity for update the reduced cost cp: O(m) where m = to the number of edges in the f
	This functions raises the prices p, as in Figure 7.4 from the paper, in oder to make all arcs epsilon-proper
	and updates the reduced cost cp for all the arcs in order to have it calculated always in the main loop;
	For the first call to this function we have cp(x, y) = c(x, y).
*/
void Raise_the_prices_as_in_7_4(const size_of_cost q, const int positions_current_epsilon){
//	cout << "Raising the prices p, as in Figure 7.4, to make all arcs epsilon-proper.\n";
	size_of_cost one = (q-1) << positions_current_epsilon, two = (2 * (q-1)) << positions_current_epsilon, 
		three = (3 * (q-1)) << positions_current_epsilon;
	
	for(int v = this->lower_women; v <= this->upper_women; v++)//first the prices for the women are increased
		if(!(virtual_source[v]&SATURATED))//if it is IDLE then is not in the set S, so is in X\S
			pd[v] += one;
	
	for(int y = this->lower_men; y <= this->upper_men; y++)//then the prices for the men are increased
		if(virtual_men[y] & SATURATED)//if it is in D
			pd[y] += three;
		else//otherwise it is IDLE then is not in the set D, so is in Y\D
			pd[y] += two;

	pd[this->source] += one;//for the source
	pd[this->sink] += two;//for the sink
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	Complexity: O(m*sqrt(s)) where m is the number of edges in the pseudoflow and s the size of the matching
	This function contains the main structure of Refine. The residual digraph is deduced from the pseudoflow.
	Input:
		s: int; the current size of the flow
*/
void Refine(const int s, const size_of_cost q, const int positions_current_epsilon, const int positions_epsilon_den_down, Vertex*& f_NG){
//	std::cout << "=============================Entering to the Refine procedure.\n";
	std::vector<int> S;
	Set_S(S);
	Set_D();
	Convert_saturated_bipartite_arcs_to_idle_in_F(S, f_NG);
	Raise_the_prices_as_in_7_4(q, positions_current_epsilon);	
	//I only want to add to the time_refine variable the spent time for the Refine procedure without the main loop
	int h = s;
	while(h > 0){
//		cout << "Current value of h = " << h << " in the main loop of Refine.\n";
		Build_forest(s, h, q, positions_current_epsilon, positions_epsilon_den_down, S, f_NG);
		h -= Finding_augmenting_paths_recursive_on_line(S, positions_current_epsilon, positions_epsilon_den_down, f_NG);
	}
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: April, 2013
	Last Update on: February, 2015
	This function allocates memory for all the neighbors of a vertex
	Input:
		v: int; the vertex reference to allocate its memory
		size: int the number of positions to allocate
*/
void Allocate_for_vertex(const int v, const int size, Vertex*& f_NG){
	f_NG[v].Allocate(size);
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: January, 2015
	This function allocates the memory for the handled arrays in Flow Assign;
*/
void Allocate_memory_for_flow_assign(){
//	cout << "Allocating memory for flow assign." << endl;
	indices_of_current_saturated_arcs = new int[this->number_vertices];
	is_a_deficit = new bool[this->number_vertices];
	for(int v = 0; v < this->number_vertices; v++)
		is_a_deficit[v] = false;
	pd = new size_of_cost[this->number_vertices];
	virtual_source = new int[this->upper_women+1];
	for(int v = this->lower_women; v <= this->upper_women; v++)
		virtual_source[v] = (married[v] != this->source ? SATURATED : 0) | v;
	virtual_men = new int[this->upper_men+1];
	for(int y = this->lower_men; y <= this->upper_men; y++)
		virtual_men[y] = this->sink | (married[y] != this->source ? SATURATED : 0);
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: January, 2015
	Last Update on: 
	This function deallocates the memory for the handled arrays in Flow Assign;
*/
void Deallocate_memory_for_flow_assign(){
//	cout << "Allocating memory for flow assign." << endl;
	delete[] virtual_men;
	delete[] virtual_source;
	delete[] indices_of_current_saturated_arcs;
	delete[] is_a_deficit;
	delete[] pd;
	return;
}


/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function returns the number of bits of 'value' minus one, which value will be used to make the left/right-shifts
	Input:
		value: size_of_cost; The value from which will be recovered the number of bits minus one
*/
inline int Get_bits(size_of_cost value){
	int number_bits;
	for(number_bits = 0; value > 0; number_bits++, value>>=1);
	return number_bits;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function returns the ceil function of the fraction numerator/denominator, but since denominator
	is a multiple of 2, we just need to do 'positions' right-shift to perform the ceiling function
	Input:
		numerator: size_of_cost; the value of the numerator from the fraction
*/
inline size_of_cost Ceil_function(const size_of_cost numerator, const int positions_current_epsilon){
	return -((-numerator)>>positions_current_epsilon);
}

/*
	Created By: Sergio Perez
	Modified by:
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function gets a value such that q^e is greater than the established value 'bound'
 	Input: 
		bound: long_cost; the required value such that value = q^e > bound
*/
size_of_cost Get_epsilon(const size_of_cost bound, const size_of_cost q){
	size_of_cost value = 1;
	while(value <= bound)
		value *= q;
	return value;
}

/*
	Created By: Sergio Perez
	Modified by:
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function check if by adding k we will be rounding up the price or the price holds
 	Input: 
		price: size_of_cost; the price to be cheked
		k: size_of_cost; the amount to add to the price
*/
inline bool round_up(const size_of_cost price, const size_of_cost k, const int positions_epsilon_den_down){
	return ((price + k) >> positions_epsilon_den_down) > (price >> positions_epsilon_den_down);
}


/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: 
	This function prints the current matching M.
*/
void Print_matching(const std::vector<std::pair<std::pair<int, int>, int> > v){
	for(unsigned int i = 0; i < v.size(); i++)
		printf("%d %d %d\n", v[i].first.first, v[i].first.second, v[i].second);
	std::cout << "\n\n";
	return;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on: 
	This function prints the current pseudoflow with the current reduced costand the status for an arc
*/
void Print_f_NG(const Vertex* f_NG){
	FILE *out = fopen("graph.txt", "w");
	fprintf(out, "%d %d\n", this->number_women, this->number_men);
	for(int v = this->lower_women; v <= this->upper_women; v++)
		for(int i = 0; i < f_NG[v].length; i++)
			fprintf(out, "%d %d %d\n",  v, (f_NG[v].neighbor[i]&VERTEX), f_NG[v].cost[i]);
	fclose(out);
	return;
}//*/

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on: April, 2013
	This function chechs the consistency of the invariants I3 and I4
	I3: Every arc of NG, idle or saturated, is epsilon-proper	
	I4: Every saturated bipartite arc is epsilon-snug
	Input:
		vertex: Arc; the arc to be checked
*/
void Verify_the_arc(const int neighbor, const size_of_cost current_epsilon, const size_of_cost cp){
	if(neighbor&SATURATED){//then is saturated
		if(cp > current_epsilon)//for saturated arcs must be proved that cp'(v, w) <= epsilon
			Message_error(20);
		if(this->lower_men <= (neighbor&VERTEX) && (neighbor&VERTEX) <= this->upper_men){
			if(cp <= (-current_epsilon))//for saturated bipartite arcs must be proved that cp'(v, w) > -epsilon
				Message_error(21);
		}
	}
	else{//for idle arcs must be proved that cp'(v, w) = c(v, w)+(i(w)-i(v))*epsilon > -epsilon
		if(cp <= (-current_epsilon))
			Message_error(22);
	}
	return;
}//*/

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function calculates the cost of the minimum cost matching
*/
long long int Get_matching_cost(const std::vector<std::pair<std::pair<int, int>, int> > &assignment){
	long long int matching_cost = 0;
	for(unsigned int i = 0; i < assignment.size(); i++)
		matching_cost += assignment[i].second;
	return matching_cost;
}

/*
	Created By: Sergio Perez
	Modified by:
	Revised By:
	Created on: March, 2013
	Last Update on: April, 2013
	This function converts M into an integral flow on NG with |f| = s.
	Since all the neighbors are in the correct position from the input and they were renamed there, 
	we do not need to do that renamed here (as in the past), now we are only looking for the match 
	from Hopcroft-karp algorithm and adding the left-dummy and right-dummy arcs.
 	Input: 
		s: int; the current size of the matching
*/
void Convert_M_into_integral_flow(const int s, int &size_flow, Vertex*& f_NG){
//	std::cout << "Converting M into an integral flow.\n";
	size_flow = 0;
	for(int x = this->lower_women; x <= this->upper_women; x++){//by default all bipartite arcs are IDLE
		for(int j = 0; j < f_NG[x].length; j++){//for each of her neighbors
			if(married[x] == (f_NG[x].neighbor[j]&VERTEX)){//if the arcs belongs to the match M
				f_NG[x].neighbor[j] |= SATURATED;//We need to mark that arc as saturated
				indices_of_current_saturated_arcs[x] = j;
				size_flow++;
			}			
		}
	}
	if(size_flow != s)
		Message_error(16);
//	std::cout << "End Converting M into an integral flow.\n";
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2015
	This function sets the prices pd (which is a generic array containing information about the women, the men, 
	and the source and the sink vertices) to the indicated 'value'
 	Input: 
		value: long_cost; the value to assign in all the prices pd
*/
void Set_prices_pd_to_value(const int number_vertices){
	for(int v = 0; v < number_vertices; v++)
		pd[v] = 0;
	return;
}

/*
	Created By: Sergio Perez
	Modified by:
	Revised By:
	Created on: March, 2013
	Last Update on:
	This function rounds the final prices in order to make all arcs proper
*/
void Round_prices_to_integers_to_get_arcs_proper(const int number_vertices, const long long int epsilon_den_down, 
	const int positions_epsilon_den_down, Vertex*& f_NG){
	int v, w;
	size_of_cost k, cp;
	bool *good = new bool[(int)epsilon_den_down+1];
	for(k = 0; k < epsilon_den_down; k++)
		good[k] = true;
//	Print_prices();
	for(int v = this->lower_women; v <= this->upper_women; v++){
		if(virtual_source[v]&SATURATED){//then is saturated
			w = f_NG[v].neighbor[indices_of_current_saturated_arcs[v]]&VERTEX;
			cp = pd[v] - pd[this->source];
			if(cp == 1){
				k = ( epsilon_den_down - (pd[v] % epsilon_den_down) ) % epsilon_den_down;
				if(k != 0 && (round_up(pd[this->source], k, positions_epsilon_den_down) || !round_up(pd[v], k, positions_epsilon_den_down)))
					Message_error(19);
				good[k] = false;
			}
			else if(cp > 1)
				Message_error(25);

			cp = (((size_of_cost)f_NG[v].cost[indices_of_current_saturated_arcs[v]]) << positions_epsilon_den_down) - pd[v] + pd[w];
			if(cp == 1){
				k = ( epsilon_den_down - (pd[w] % epsilon_den_down) ) % epsilon_den_down;
				if(k != 0 && (round_up(pd[v], k, positions_epsilon_den_down) || !round_up(pd[w], k, positions_epsilon_den_down)))
					Message_error(19);
				good[k] = false;
			}
			else if(cp > 1)
				Message_error(25);
		}
	}

	good[k = epsilon_den_down - (pd[this->sink] % epsilon_den_down)] = false;

	for(k = 0; k < epsilon_den_down && !good[k]; k++);

	for(v = 0; v < number_vertices; v++)
		pd[v] = (pd[v] + k) >> positions_epsilon_den_down;

	for(v = lower_women; v <= upper_women; v++){
		for(int i = 0; i < f_NG[v].length; i++){
			if(f_NG[v].neighbor[i]&SATURATED){
				if((f_NG[v].cost[i] + pd[f_NG[v].neighbor[i]&VERTEX]) > pd[v])
					Message_error(17);
			}
			else if((f_NG[v].cost[i] + pd[f_NG[v].neighbor[i]]) < pd[v])//if it is idle then
					Message_error(18);
		}
	}//*/
	delete[] good;//*/
	return;
}

void Re_indexing(const int sign, Vertex* f_NG){
	for(int x = this->lower_women; x <= this->upper_women; x++)
		for(int y = 0; y < f_NG[x].length; y++)
			f_NG[x].neighbor[y] += (this->lower_men*sign);
	return;
}

/*
	Created By: Sergio Perez
	Modified by: Sergio Perez
	Created on: March, 2013
	Last Update on: February, 2013
	This function is the main structure of flow assign algorithm;
 	Input: 
		t: int; the size of the required matching
*/
std::vector<std::pair<std::pair<int, int>, int> > Flow_assign(int t, const int numberWomen, const int numberMen,
	Vertex*& f_NG, const size_of_cost q = 4){
//	std::cout << "Flow Assign is running.\n";
	this->number_women = numberWomen, this->number_men = numberMen;
	this->Set_indices(this->number_women, this->number_men);
	this->DetermineValueC_up(f_NG);
	long long int epsilon_up, epsilon_num_down;
	int size_flow = 0;

	married = new int[this->number_vertices];

//	this->Print_f_NG(f_NG);

	Re_indexing(1, f_NG);

	HopcroftKarp hop;
	int s = hop.Hopcroft_Karp(t, number_women, number_men, married, f_NG);

	Allocate_memory_for_flow_assign();
	//this value is calculated here, in order to allocate the maximum amount of memory to use for the heap

	epsilon_num_down = 1;
	size_of_cost epsilon_den_down = Get_epsilon(s+2, q);//this is calculated here in order to multiplied the costs by this denominator
	int positions_epsilon_den_down = Get_bits(epsilon_den_down) - 1;//to denote the number of shifts to perform

	Convert_M_into_integral_flow(s, size_flow, f_NG);
	Set_prices_pd_to_value(number_vertices);

	epsilon_up = Get_epsilon(C_up, q);
	size_of_cost current_epsilon = epsilon_up << positions_epsilon_den_down;

//	std::cout << "q = "<<q <<" epsilon_up = " << epsilon_up << " s+2 = "<< (s+2) << "\n";
//	std::cout << "First value of epsilon = " << current_epsilon << " and epsilon_den_down = " << epsilon_den_down << "\n";

	while(current_epsilon > epsilon_num_down){
		current_epsilon = current_epsilon / q;
		int positions_current_epsilon = Get_bits(current_epsilon) - 1;
//		std::cout << "Current epsilon = " << current_epsilon << 	" shifts to perform " << positions_current_epsilon << "\n";
		Refine(s, q, positions_current_epsilon, positions_epsilon_den_down, f_NG);
	}

//	std::cout << "epsidown " << epsilon_den_down << "\n";
	Round_prices_to_integers_to_get_arcs_proper(number_vertices, epsilon_den_down, positions_epsilon_den_down, f_NG);

	std::vector<std::pair<std::pair<int, int>, int> > assignment;

	for(int v = this->lower_women; v <= this->upper_women; v++){
		if(virtual_source[v]&SATURATED){
			int y = (f_NG[v].neighbor[indices_of_current_saturated_arcs[v]]&VERTEX)-this->lower_men;
			assignment.push_back(std::make_pair (std::make_pair (v, y), f_NG[v].cost[indices_of_current_saturated_arcs[v]]));
			f_NG[v].neighbor[indices_of_current_saturated_arcs[v]] &= VERTEX;//recovering the structure of the original graph
		}
	}

	Re_indexing(-1, f_NG);
	Deallocate_memory_for_flow_assign();
	delete[] married;
//	std::cout << "Flow Assign has finished.\n";
	return assignment;
}

};

#endif