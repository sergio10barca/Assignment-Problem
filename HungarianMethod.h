#ifndef HUMGARIANMETHOD
#define HUMGARIANMETHOD
#include <set>
#include <utility>
#include <vector>
#include <iostream>
#include <climits>
#include <algorithm>
#include <cstdio>

//the first element is the priority and the second one the key, used for the heap

class Edge{
public:
	int vertex, father, relative_index_in_G;
	Edge(int v = 0, int f = 0, int r = 0){
		vertex = v;
		father = f;
		relative_index_in_G = r;
	};
	~Edge(){
		vertex = father = this->relative_index_in_G = 0;
	}
	bool operator<(const Edge& e) const{
		if(vertex < e.vertex)
			return true;
		return false;
	}
};

	#define x first
	#define y second

class Hungarian{
private:

	int C_up;
	int lower_men, lower_women, upper_men, upper_women, source;
	int *indices;
	int *married;
	int *indices_of_saturated_arcs;
	long long int *prices;
	long long int *l_value;
	Edge *forest;
	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function raises the prices and updates only the reduced cost
		The prices are updated in O(n+r) time
		The reduced cost matrix is updated in O(m) time, but we only must update
		the reduced cost values for the modified prices
	*/
	void raisePrices(const int forest_size, const int Beta){
		for(int i = 0; i < forest_size; i++)
			prices[forest[i].vertex] += (l_value[Beta]-l_value[forest[i].vertex]);
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function deletes and recovers the key with the less value in the heap
		Takes O(1) time (For the version (erase(position)), amortized constant.)
		Reference http://www.cplusplus.com/reference/set/set/erase/
	*/
	std::pair<long long int, Edge> deleteMin(std::set<std::pair<long long int, Edge> > &heap){
		std::pair<long long int, Edge> p = *heap.begin();//Constant time
		heap.erase(heap.begin());//amortized constant
		return p;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function inserts a new pair (key, value) in the heap as (y, L)
		Takes O(log(sizeof(Heap))) time -> O(log(r))
	-- 	Parameters
		y is the man to insert in the forest
		L is the value to set for y
		x is the node related to y with value L
	*/
	void insertHeap(const int y, const long long int L, const int x, const int index, 
		std::set<std::pair<long long int, Edge> >::iterator*& pointers_to_heap, std::set<std::pair<long long int, Edge> > &heap){

		std::pair<std::set<std::pair<long long int, Edge> >::iterator, bool> result = heap.insert(std::make_pair(L, Edge(y, x, index)));
		if(result.second == false){
			std::cout << "Some value could not be inserted in the heap.\n";
			exit(32);
		}
		pointers_to_heap[y] = result.first;
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function updates the value for the key element in the heap
		Takes O(log(sizeof(Heap))) time -> O(log(r))
	--	Parameters
		y is the key value we want to decrease
		L is the new value to set y
		x is the new node related to y with value L
		---For better implementation
		As I see, by using a map structure and saving the iterator, this operation can be done in constant time
	*/
	void decreaseKey(const int y, const long long int L, const int x, const int index, 
		std::set<std::pair<long long int, Edge> >::iterator*& pointers_to_heap, std::set<std::pair<long long int, Edge> > &heap){
		heap.erase(pointers_to_heap[y]);//amortized constant
		insertHeap(y, L, x, index, pointers_to_heap, heap);//log in the size fo the heap
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function builds the forest
		The array indices told to the forest which is the index value in the forest for
		recovering the connected information for the vertex (either man or woman depending
		on the its sign)
		IMPORTANT: So in the forest structure a man always is denoted with a negative value
		and a woman always is denoted with a positive value
		Parameters
		vertex is the element to insert predecessor 
		Noticed that if predecessor is zero then the current element is a root
		Takes O(1) time
	*/
	void AddForest(const int vertex, const int father, const int index, int &forest_size){
		indices[vertex] = forest_size;//index for the woman in the forest structure 
		forest[forest_size++] = Edge(vertex, father, index);//father of the current element
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function updates the shortest values for each vertex which is not yet 
		in the current forest.
	--	Parameters 
		x is the man who we want to update his costs to his neighbors
		predecessor is the father of x, if predecessor is 0 then x is a root-tree
		Takes O(r*log(r))
	*/
	void ScanAndAdd(const int x, const int father, const int related_index, int &forest_size, 
		std::set<std::pair<long long int, Edge> >::iterator*& pointers_to_heap, std::set<std::pair<long long int, Edge> > &heap, const Vertex* G){// x is always a woman

		long long int L, Lold;
		//for all forward links x => y in R_M
		for(int i = 0, y = 0; i < G[x].length; i++){
			if(married[y = (G[x].neighbor[i] + this->lower_men) ] != x){//is forward?
				L = l_value[x]+(G[x].cost[i] - prices[x] + prices[y]);
				Lold = l_value[y];

				if(L < 0){
						std::cout << "Error: L = " << L << "\n";
						std::cout << "l_value["<<x<<"] = " << l_value[x]<<"\n";
						std::cout << "G[x].cost["<<i<<"] = "<<G[x].cost[i] << "\n";
						std::cout << "x = " << x <<", y = " << y <<" original y "<< y<<"\n";
						std::cout << "price[y] = " << prices[y] << "\n";
						std::cout << "price[x] = " << prices[x] << "\n";
						FILE *graph = fopen("error_graph.txt", "w");
						if(graph == NULL){
							std::cout << "The graph could not be generated.\n";
							exit(0);
						}
						fprintf(graph, "%d %d\n", this->upper_women-this->lower_women+1, this->upper_men-this->lower_men+1);
						for(int a = this->lower_women; a <= this->upper_women; a++)
							for(int b = 0; b < G[a].length; b++)	
								fprintf(graph, "%d %d %d\n", a, G[a].neighbor[b], G[a].cost[b]);
						fclose(graph);
						exit(0);
				}
				if(L < Lold){
					l_value[y] = L;
					//'x' allows to have in mind that 'y' is currently related to 'x'
					if(Lold == INT_MAX)
						insertHeap(y, L, x, i, pointers_to_heap, heap);
					else
						decreaseKey(y, L, x, i, pointers_to_heap, heap);
				}
			}
		}
		//set predecessor as father of x in the forest, it means the edge (predecessor->x)
		//is added to the forest, if predecessor is 0 then means that x is a root-tree
		AddForest(x, father, related_index, forest_size);
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This Function the BuildForest with roots at all the current maidens
	*/
	bool BuildForest(int &Beta, const Vertex* G){
		//make-heap
		std::set<std::pair<long long int, Edge> > heap;
		std::set<std::pair<long long int, Edge> >::iterator *pointers_to_heap = new std::set<std::pair<long long int, Edge> >::iterator[this->upper_men+1];

		bool the_forest_was_build = false;
		std::pair<long long int, Edge> Y;

		int forest_size = 0;
		//forall nodes v, set l(v) = INFINITY
		for(int x = this->lower_women; x <= this->upper_women; x++)
			l_value[x] = INT_MAX;
		for(int y = this->lower_men; y <= this->upper_men; y++)
			l_value[y] = INT_MAX;
		//forall maidens mu, set l(mu) = 0 and ScanAndAdd(mu)
		for(int x = this->lower_women; x <= this->upper_women; x++){
			if(married[x] == this->source){//if x does not have husband (represented by source) then she is maiden
				l_value[x] = 0;
				ScanAndAdd(x, this->source, this->source, forest_size, pointers_to_heap, heap, G);
			}
		}
		//PrintForest();
		while(heap.size()>0){
			Y = deleteMin(heap);
//			printf("%lld %d %d\n", Y.first, Y.second.vertex, Y.second.father);
			//remember it!! man should be passed as a negative value to add y to the forest 
			AddForest(Y.second.vertex, Y.second.father, Y.second.relative_index_in_G, forest_size);
			int x = married[Y.second.vertex];//if x is a value greater than 0 then Y has a wife
			if(x != this->source){//it is married 
//				printf("y = %d is married to x = %d\n", Y.second.vertex, x);
				l_value[x] = l_value[Y.second.vertex];//l(x) = l(y)
				
				ScanAndAdd(x, Y.second.vertex, Y.second.relative_index_in_G, forest_size, pointers_to_heap, heap, G);//'Y' will be the father of 'x'
			}
			else{			
				Beta = Y.second.vertex;
//				printf("**A Bachelor y = %d was reached with cost l(beta) = ", Beta);
				the_forest_was_build = true;//bachelor Beta = y reached
				break;
			}
		}
		if(Beta >= 0)
			raisePrices(forest_size, Beta);//raise prices to tighten the tree path to Beta

		delete[] pointers_to_heap;
		heap.clear();
		return the_forest_was_build;//no bachelor reached
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
	*/
	void Set_M_to_the_emtpy_matching(){
		for(int v = this->lower_men; v <= this->upper_men; v++)
			married[v] = this->source;
		for(int v = this->lower_women; v <= this->upper_women; v++)
			married[v] = this->source;
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
	*/
	void Set_prices(){
	//	cout << "Setting the value for the prices" << endl;
		//set prices at women to 0
		for(int v = this->lower_women; v <= this->upper_women; v++)
			prices[v] = 0;
		//set prices at men to C_up
		for(int v = this->lower_men; v <= this->upper_men; v++)
			prices[v] = C_up;
		return;
	}

	/*
		This function augments the path along the current match, based on the built forest
	*/
	void AugmentM(const int Beta){
		int previous_man = 0, vertex = Beta;
		Edge edge;
//		printf("Augmenting from the Bachelor %d\n", Beta);
		while(vertex != this->source){
			if(this->lower_men <= vertex && vertex <= this->upper_men){//IS a MAN
	//			printf("is a man\n");
				edge = forest[indices[vertex]];//recovering the related edge
				married[vertex] = edge.father;//vertex has a new wife
				previous_man = vertex;
				vertex = edge.father;//now we must update the father in the augmenting path
				indices_of_saturated_arcs[edge.father] = edge.relative_index_in_G;
			}
			else{//IS a WOMAN		
	//			printf("is a woman\n");	
				edge = forest[indices[vertex]];//recovering the related edge
				married[vertex] = previous_man;//this woman has husband (maybe either a new or another one)			
				vertex = edge.father;
			}
		}
		return;
	}

	void DetermineValueC_up(const Vertex* f_NG){
		C_up = 0;
		for(int v = this->lower_women; v <= this->upper_women; v++)
			for(int j = 0; j < f_NG[v].length; j++)
				C_up = f_NG[v].cost[j] < 0 ? std::max(C_up, -f_NG[v].cost[j]) : std::max(C_up, f_NG[v].cost[j]);
		return;
	}

public:
	void Add_arc(int x, int y, int cost, Vertex*& G){
		G[x].neighbor[G[x].length] = y;
		G[x].cost[G[x].length] = cost;
		G[x].length++;
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function sets the values for the allowed indices in the generic arrays (for women, men, source and sink );
		is better does not use the position 0 for some woman or man since 0 is considered as special value in some cases
	*/
	void Set_indices(int &number_vertices, const int number_women, const int number_men){
		number_vertices = number_women+number_men+1;// we must consider at the begining the index and the source
		this->lower_women = 0;
		this->upper_women = number_women-1;
		this->lower_men = number_women;
		this->upper_men = number_women+number_men-1;
		this->source = number_women+number_men;
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function allocates the main memory to save the graph G in Woman and the costs in the arcs in edge_cost
	*/
	void Allocate_main_memory(int number_women, int number_men, Vertex*& G){
	//	cout << "Allocating the main memory used for the program." << endl;
		G = new Vertex[number_women];
		for(int i = 0; i < number_women; i++)
			G[i].Allocate(number_men);
		return;
	}

	/*
		Created By: Sergio Perez
		Modified by: Sergio Perez
		Created on: March, 2013
		Last Update on: January, 2015
		This function calculates the cost of the minimum cost matching
	*/
	std::vector<std::pair<std::pair<int, int>, int > > Get_matching_cost(const Vertex* G){
		std::vector<std::pair<std::pair<int, int>, int > > matching;
		for(int x = this->lower_women; x <= this->upper_women; x++)
			if(this->lower_men <= married[x] && married[x] <= this->upper_men)
				matching.push_back(std::make_pair(std::make_pair (x, married[x]-this->lower_men), G[x].cost[indices_of_saturated_arcs[x]]));
		return matching;
	}
	/*
		The main structure for the Hungarian Method
	*/
	std::vector<std::pair<std::pair<int, int>, int> > HungarianMethod(const int s, const int number_women, const int number_men, const Vertex* G){
//		cout << "The hungarian method is starting" << endl;
		int number_vertices;

		Set_indices(number_vertices, number_women, number_men);
		this->DetermineValueC_up(G);

		l_value = new long long int[number_vertices];
		prices = new long long int[number_vertices];
		indices = new int[number_vertices];
		married = new int[number_vertices];
		indices_of_saturated_arcs = new int[this->upper_women+1];
		forest = new Edge[number_vertices];

		Set_M_to_the_emtpy_matching();
		Set_prices();
	
		for(int size_match = 0; size_match < s; size_match++){
//			printf("===============(Iteration %*d)===============\n", 5, size_match);
			int Beta = -1;
			//using Dijkstra to build a shortest path forest with roots at all remaining maidens
			BuildForest(Beta, G);
			if(Beta >= 0){//some bachelor was reached			
				AugmentM(Beta);//augment M along that tight path
			}
			else{
				std::cout << "nu(G) = " << size_match << "\n";
				break;
			}
		}

		std::vector<std::pair<std::pair<int, int>, int> > v = Get_matching_cost(G);

		delete[] l_value;
		delete[] prices;
		delete[] indices;
		delete[] married;
		delete[] indices_of_saturated_arcs;
		delete[] forest;
		return v;
	}
};

#endif