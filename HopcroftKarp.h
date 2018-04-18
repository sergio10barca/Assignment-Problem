#ifndef HOPCROFTKARP
#define HOPCROFTKARP
#include <climits>
#include <queue>
#include <vector>
#include <utility>

class HopcroftKarp{

private:
	int lower_women, upper_women, source;
	int *married;
	int *dist_woman;

public:
	HopcroftKarp(){
		return;
	}
/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on: April, 2013
	Complexity: O(m)  where m is the number of edges in the graph
	This function finds a paths of minimal length, which is saved in DistWoman[0]
	The vector Dist Woman allows to detect the augmenting path of minimum length and it will 
	be used to make the augmentations, if no augmention path is found then DistWoman[0] = INF
*/
bool BFS(Vertex*& f_NG){
	std::queue<int> q;//This queue allows to process the women
	int x, y;
	for(x = this->lower_women; x <= this->upper_women; x++){
		if(married[x] == this->source){
			dist_woman[x] = 0;
			q.push(x);
		}
		else
			dist_woman[x] = INT_MAX;
	}
	//At the end, if DistWoman[0] != 0 then it represents the quantity of idle arcs in the path
	dist_woman[this->source] = INT_MAX;
	while(!q.empty()){		
		x = q.front();//recovering the woman in front of the queue
		q.pop();//extracting such a woman
		if(dist_woman[x] < dist_woman[this->source]){
			for(int i = 0; i < f_NG[x].length; i++){//for each man related to the woman x
				if(dist_woman[married[y = f_NG[x].neighbor[i]]] == INT_MAX){
					dist_woman[married[y]] = dist_woman[x]+1;
					q.push(married[y]);//we must analyze the neighbors of this woman
				}
			}
		}
	}
//	PrintDistances();
	return dist_woman[this->source] != INT_MAX;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on: April, 2013
	Complexity: O(m)  where m is the number of edges in the graph
	For the vertex x, we are looking for its trajectory of augment of size minimum based on the calculated distances
*/
bool DFS(const int x, Vertex*& f_NG){
	int y;
	if(x != this->source){
		for(int i = 0; i < f_NG[x].length; i++){
			//we must consider only possible candidates of the size we want to reach
			if(dist_woman[married[y = f_NG[x].neighbor[i]] ] == (dist_woman[x] + 1)){
                if(DFS(married[y], f_NG)){//augment was found
//                	cout << setw(weight_word) << x <<"->" <<setw(weight_word) << y << endl;
                    married[y] = x, married[x] = y, dist_woman[x] = INT_MAX;
                    return true;
				}
			}
		}
        dist_woman[x] = INT_MAX;//we unmark the current vertices in order to avoid future innecesary iterations
        return false;
	}
    return true;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	Complexity: O(m) where m is the number of edges in the graph
	For the vertex x, we are looking for its trajectory of augment of size minimum	
	based on calculated distances
*/
bool DFS_stack(int x, Vertex*& f_NG){
	std::vector<std::pair<int, int> > sp;
	int y, index;
	sp.push_back(std::make_pair(f_NG[x].length, 0));
	married[this->source] = x;	
	while(sp.size() > 0){
		index = (int)sp.size()-1;//****From Figure 8.2 v := top(K)
		x = married[sp[index].second];
		if(x == this->source)//****From Figure 8.2 if v in D
			break;
//		cout << "Current in DFS = " << x << endl;
		while(sp[index].first > 0){//****From Figure 8.2 while L[v] not empty do
			sp[index].first--;
			if(dist_woman[married[y = f_NG[x].neighbor[sp[index].first]] ] == (dist_woman[x] + 1)){
				sp.push_back(std::make_pair(f_NG[married[y]].length, y));
				break;
			}
		}
		if(sp[index].first < 0){//****From Figure 8.2 pop(K)
			dist_woman[x] = INT_MAX;
			index = (int)sp.size();
			sp.erase(sp.begin()+(index-1));
		}
	}
	if(sp.size() == 0){//****From Figure 8.2 if v in D then sp.size will be > 0
		return false;
	}
	for(index = (int)sp.size()-1; index > 0; index--){//****From Figure 8.2 add K to P as an augmenting path
		x = married[sp[index-1].second], y = sp[index].second;
		married[y] = x, married[x] = y;
		dist_woman[x] = INT_MAX;
	}
	sp.clear();
    return true;
}//*/

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	Complexity: O(|X|+|Y|) 
	This function initializes the matching, which means that at the beginning there are no vertices married
*/
void Set_to_empty_matching(const int number_vertices){
	for(int v = 0; v < number_vertices; v++)
		married[v] = this->source;
	return;
}

void Set_indices(const int number_women, const int number_men, int &number_vertices){
	number_vertices = number_women+number_men+1+1;// we must consider at the begining the index and the source
	this->lower_women = 0;
	this->upper_women = number_women-1;
	this->source = number_women+number_men;//the index for the vertex source
	return;
}

/*
	Created By: Sergio Perez
	Modified by: 
	Revised By:
	Created on: March, 2013
	Last Update on:
	Complexity: O(m*sqrt(s)) 
	Main structure of Hopcroft-Karp algorithm.
*/
int Hopcroft_Karp(int s, const int number_women, const int number_men, int*& married, Vertex*& f_NG){
//	std::cout << "Hopcroft-Karp algorithm is looking for  a match of size t = " << s << "\n";
	int number_vertices;

	Set_indices(number_women, number_men, number_vertices);

	int x;
	dist_woman = new int[number_vertices];
	this->married = married;
	int iteration = 0, size_of_matching = 0;

	Set_to_empty_matching(number_vertices);
	for(iteration = 0; size_of_matching < s; iteration++){
//		cout << "=====The current iteration of Hopcroft-Karp is "<<iteration<<"======" << endl;
		if(!BFS(f_NG)){//if no augmenting
//			cout << "We have reached v(G) = " << size_of_matching << endl;
			break;
		}
		for(x = this->lower_women; x <= this->upper_women && size_of_matching < s; x++){
			if(this->married[x] == this->source){//if the woman x does not have yet husband then...
				if(DFS(x, f_NG))//only if the length matches with the minimum possible length path
					size_of_matching++;
			}
		}
	}
//	std::cout << "We reach a match of size s = " << size_of_matching << ".\n";
	delete[] dist_woman;
	this->married = NULL;
	return size_of_matching;
}

};

#endif