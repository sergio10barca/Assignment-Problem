#ifndef _T_FIBBONACCI_HEAP_
#define _T_FIBBONACCI_HEAP_

#include <iostream>
#include <new>
#include <cmath>

//#include "error.h"

using std::cout;
using std::nothrow;

//********************************************************************************
//********************************************************************************
/*
    ***************
    * DESCRIPTION *
    ***************
    
    
    
    *****************
    * INSTANTIATION *
    *****************
    
    
    
    ***********
    * MEMBERS *
    ***********
    
    
    
    ***********
    * METHODS *
    ***********
    
    
    
    ***********************
    * INTERN OVERLOADINGS *
    ***********************
    
    
    
    ***********************
    * EXTERN OVERLOADINGS *
    ***********************
    
    
    
    **************
    * DESTRUCTOR *
    **************
 
    
*/
//********************************************************************************

//A node with data and four linking pointers.
template <typename Tdata>
struct T4linknode
{
	//The assignment operator.
	T4linknode& operator=(const T4linknode& T4L)
	{
		mUp=T4L.mUp;
		mDown=T4L.mDown;
		mLeft=T4L.mLeft;
		mRight=T4L.mRight;
		mData=T4L.mData;
	}
	
    //THE CONSTRUCTOR, wich initialize the members to default values.
    T4linknode()
    {
        mUp=0;
		mDown=0;
		mLeft=0;
		mRight=0;
    }
	
	//Members
    //The element stored in the node.
    Tdata			mData;
    //The four links.
    T4linknode*		mUp;
	T4linknode*		mDown;
	T4linknode*		mLeft;
	T4linknode*		mRight;
};


//Tsize represents the capacity of the number of children.
template <typename Tsize, typename Tdata>
struct FH_data
{
	//The assignment operator.
	FH_data& operator=(const FH_data& FHD)
	{
		mKey=FHD.mKey;
		mChildren=FHD.mChildren;
		mMarked=FHD.mMarked;
	}
	
	//THE CONSTRUCTOR, wich initialize the members to default values.
	FH_data()
	{
		mChildren=0;
		mMarked=false;
	}
	
	//The Key.
	Tdata	mKey;
	//The nomber of children.
	Tsize	mChildren;
	//The marked state.
	bool	mMarked;
};


template<typename Tsize, typename Tdata>
struct Tfibonacciheap
{
	typedef		T4linknode< FH_data<Tsize, Tdata> >		FH_node;
	
	
	//Under the asumption that "root" points to a valid root (not the one with the minimum key), 
	//this method unlinks the node pointed by "root" from the root set.
	void	unlink(FH_node* root)
	{
		if((mBegin==0)||(root==0)) return;
		
		if(mRootSize==1)
		{
			mBegin=0;
			return;
		}
		
		root->mLeft->mRight=root->mRight;
		root->mRight->mLeft=root->mLeft;
		root->mLeft=root;
		root->mRight=root;
	}
	
	
	//Given the pointers to 2 lists of "roots", merges the second list to the first list.
	//The parameters are given by reference and will be modified only if some of the two are null pointers.
	void	merge(FH_node*& FH1, FH_node*& FH2)
	{
		FH_node*	aux_pointer(0);
		
		////Consider the case where one of the heaps is empty.
		if(FH2==0)
		{
			FH2=FH1;
		}
		else if(FH1==0)
		{
			FH1=FH2;
		}
		else
		{
			//We need to point this address auxiliary.
			aux_pointer=FH1->mRight;
			//Adjust the pointers.
			FH1->mRight->mLeft=FH2;
			FH1->mRight=FH2->mRight;
			FH2->mRight->mLeft=FH1;
			FH2->mRight=aux_pointer;
		}
		
		aux_pointer=0;
	}
	
	
	//Merge a fibonacci heap whith this one. Updates the pointer to the minimum key.
	void	merge(FH_node* FH2, Tsize numRoots=0, Tsize numNodes=0)
	{
		if((mBegin==0)&&(FH2==0)) return;
		
		merge(mBegin, FH2);
		
		//Point to the new minimum.
		if(FH2->mData.mKey < mBegin->mData.mKey) mBegin=FH2;
		
		//Update the number of elements.
		mRootSize+=numRoots;
		mSize+=numNodes;
		
		FH2=0;
	}
	
	
	//Insert a new node in the heap.
	void	insert(Tdata data)
	{
		//Create the new node.
		FH_node*	aux_node(new (std::nothrow) FH_node);
		
		//Make sure that the memory was allocated.
		//if(aux_node==0) throw error(memoryAllocationError, "In the 'insert' method of the 'Tfibonacciheap' class.");
		if(aux_node==0) throw std::string("memoryAllocationError, in the 'insert' method of the 'Tfibonacciheap' class.");
		
		//Put the data in the new node.
		aux_node->mData.mKey=data;
		
		//Consider the case where the heap is empty.
		if(mRootSize==0)
		{
			aux_node->mLeft=aux_node;
			aux_node->mRight=aux_node;
			mBegin=aux_node;
		}
		else
		{
			aux_node->mLeft=mBegin;
			aux_node->mRight=mBegin->mRight;
			mBegin->mRight=aux_node;
			aux_node->mRight->mLeft=aux_node;
		}
		
		//Update the number of elements.
		mRootSize++;
		mSize++;
		
		//Point to the new minimum.
		if(data < mBegin->mData.mKey) mBegin=aux_node;
		
		aux_node=0;
	}
	
	
	//Delete and return the minimum key.
	Tdata	deleteMin()
	{
		//Consider the case where the heap is empty.
		//if(mRootSize==0) throw error(emptyListError, "While removing the first element in the 'deleteMin' method of the 'Tfibonacciheap' class.");
		if(mRootSize==0) throw std::string("emptyListError, while removing the first element in the 'deleteMin' method of the 'Tfibonacciheap' class.");
		
		Tdata		minKey(mBegin->mData.mKey);
		//We must process all the children of the begin node.
		FH_node*	aux_pointer(mBegin->mDown);
		
		for(register Tsize cnt_child=0; cnt_child < mBegin->mData.mChildren; cnt_child++)
		{
			aux_pointer->mUp=0;
			aux_pointer->mData.mMarked=false;
			aux_pointer=aux_pointer->mRight;
		}
		
		//Save the address of the begin node.
		aux_pointer=mBegin;
		
		//Consider the case where the begin node has no children.
		if(aux_pointer->mData.mChildren > 0)
		{
			//Merge the children of the begin node with the root list.
			merge(mBegin, aux_pointer->mDown);
			//Point to the new begin.
			mBegin=aux_pointer->mDown;
			aux_pointer->mDown=0;
		}
		else mBegin=mBegin->mRight;
			
		//Unlink the old begin node.
		aux_pointer->mLeft->mRight=aux_pointer->mRight;
		aux_pointer->mRight->mLeft=aux_pointer->mLeft;
		aux_pointer->mLeft=0;
		aux_pointer->mRight=0;
		
		//Update the sizes.
		mRootSize+=(aux_pointer->mData.mChildren-1);
		mSize--;
		
		//Delete the old begin node.
		delete aux_pointer;
		aux_pointer=0;
		
		//We have the following two special cases.
		if(mRootSize==0)
		{
			mBegin=0;
			return minKey;
		}
		else if(mRootSize==1) return minKey;
		
		//If we reach this point, then we have mBegin pointing to a Fibonacci-Heap with at least two roots.
		
		FH_node**	degreeArray(0);
		Tsize		len((Tsize)((double)3.520781963*log((double) mSize))+(Tsize)2);
		FH_node*	equalDegree(0);
		Tsize		numRoots(mRootSize);
		
		degreeArray=new (nothrow) FH_node*[len];
		
		//If there is no sufficient memory for the auxiliary array, then just return and we will take care 
		//of merging roots with equal degree when there is sufficient memory.
		//This will not affect the structure of the fibonacci heap.
		if(degreeArray==0) return minKey;
		
		for(register Tsize cnt=0; cnt<len; cnt++) degreeArray[cnt]=0;
		//aux_pointer=mBegin;
		
		//We will scan the nodes from mBegin to the end (mBegin->mLeft). Since we can only merge nodes that have already been scaned.
		//This help us to have presition in the number of interations.
		for(register Tsize cnt_roots=0; cnt_roots<numRoots; cnt_roots++)
		{
			//At the begining of each iteration, "aux_pointer" will point to the node that we will process. And "mBegin" to the nex node.
			aux_pointer=mBegin;
			mBegin=mBegin->mRight;
			//Point to the node with the same number of children (degree) than the node that we are processing.
			equalDegree=degreeArray[aux_pointer->mData.mChildren];
			
			//We must merge until there is no nodes with the same degree after a merge.
			//At the begining of the loop, we have the invariant that "aux_pointer" will point to the root that we are processing (which may change after a merge),
			//and "equalDegree" will point to the node that has the same degree than the node that we are processing.
			while(equalDegree!=0)
			{
				//Since we will merge the two roots with the same degree, then there will be no more processed nodes with this degree.
				degreeArray[aux_pointer->mData.mChildren]=0;
				//Consider which node has the minimum key. At the end we make sure that "aux_pointer" points to the node in the top.
				if(equalDegree->mData.mKey < aux_pointer->mData.mKey)
				{
					unlink(aux_pointer);
					merge(equalDegree->mDown,aux_pointer);
					//Make sure that each node points to its child with the minimum key among all its children.
					/*if(aux_pointer->mData.mKey < equalDegree->mDown->mData.mKey)
						equalDegree->mDown=aux_pointer;*/
					//Update the parent of the node.
					aux_pointer->mUp=equalDegree;
					//Make sure that "aux_pointer" points to the top.
					aux_pointer=equalDegree;
				}
				else
				{
					unlink(equalDegree);
					merge(aux_pointer->mDown,equalDegree);
					//Make sure that each node points to its child with the minimum key among all its children.
					/*if(equalDegree->mData.mKey < aux_pointer->mDown->mData.mKey)
						aux_pointer->mDown=equalDegree;*/
					//Update the parent of the node.
					equalDegree->mUp=aux_pointer;
					//Since "aux_pointer" has the minimum key, the we don't need to change its value.
				}
				
				//Update the number of roots.
				mRootSize--;
				//Increase the number of children of the top node.
				aux_pointer->mData.mChildren++;
				//Get the following node with the same degree as the top node.
				equalDegree=degreeArray[aux_pointer->mData.mChildren];
			}
			//Register the new node in the array of degrees.
			degreeArray[aux_pointer->mData.mChildren]=aux_pointer;
		}
		
		equalDegree=0;
		delete[] degreeArray;
		degreeArray=0;
		
		//At the end, "mBegin" may be pointing to the children of a root. We make it point to a root.
		mBegin=aux_pointer;
		
		//Its time to find the node with the minimum key.
		
		//"aux_pointer" will be the scanning pointer and "mBegin" will be pointing to the minimum.
		//Note that "aux_pointer" is pointing to mBegin.
		for(register Tsize cnt_roots=0; cnt_roots<mRootSize; cnt_roots++)
		{
			if(aux_pointer->mData.mKey < mBegin->mData.mKey)
				mBegin=aux_pointer;
			aux_pointer=aux_pointer->mRight;
		}
		aux_pointer=0;
		
		return minKey; 
	}
	
	
	//Returns the minimum value stored in the heap.
	Tdata	getMin()
	{
		//if(mRootSize==0) throw error(emptyListError, "While requesting the minimum key in the 'getMin' method of the 'Tfibonacciheap' class.");
		if(mRootSize==0) throw std::string("emptyListError, while requesting the minimum key in the 'getMin' method of the 'Tfibonacciheap' class.");
		
		else return mBegin->mData.mKey;
	}
	
	
	//Decreases the key of a node to the given value.
	void	decreaseKey(FH_node* node, Tdata newKey)
	{
		//We don't allow newKey to be greater than the key of the node.
		if((node == 0)||(node->mData.mKey < newKey)) return;
		
		//Keep track of the parent.
		FH_node*	parent(node->mUp);
		FH_node*	minimum(node);
		
		//Update the node with the new key.
		node->mData.mKey=newKey;
		
		//Consider the case where the heap condition is not violated (only if the node is not a root).
		if((parent!=0)&&(newKey < parent->mData.mKey))
		{
			while(parent!=0)
			{
				//Make sure that, if posible, the parent is not pointing to the node that we will unlink.
				if(parent->mDown == node) parent->mDown=node->mRight;
				
				//Consider the case where this is the only child.
				if(parent->mData.mChildren == 1)
				{
					parent->mDown=0;
				}
				else
				{
					//Unlink the node.
					node->mLeft->mRight=node->mRight;
					node->mRight->mLeft=node->mLeft;
				}
				
				//Make this single node double link to itself.
				node->mLeft=node;
				node->mRight=node;
				//All roots has no parent, and are no marked.
				node->mUp=0;
				node->mData.mMarked=false;
				
				//Merge this node with the root list.
				merge(mBegin,node);
				//Decrease the number of children of the parent and increase the number of roots.
				parent->mData.mChildren--;
				mRootSize++;
				
				//Consider the case where the parent is marked or not.
				if(parent->mData.mMarked == true)
				{
					//Since the next node that we will process is the parent, we point to it to maintain the invariant.
					node=parent;
					//If the node was marked then we point parent to the parent of the new node and update the pointer to the minimum.
					parent=node->mUp;
					if(node->mData.mKey < minimum->mData.mKey) minimum=node;
				}
				else
				{
					//Mark the parent only if it is not a root.
					if(parent->mUp!=0) parent->mData.mMarked=true;
					//If the parent was not markes then we have finished the cascade.
					parent=0;
				}
			}
		}
		
		//Since we have "minimum" pointing to the minimum of the removed nodes, and mBegin to the minimum of the original roots, 
		//then we can easily update the mBegin pointer.
		if(minimum->mData.mKey < mBegin->mData.mKey) mBegin=minimum;
		
		node=0;
		parent=0;
		minimum=0;
	}
	
	
	//Removes from the FH the node pointed by the parameter.
	void	deleteNode(FH_node* node)
	{
		//First decrease the key of the node to a value that tightly guaratees that will be the node with the minimum key.
		//WARNING: We probably need to overload the "-" operator, in Tdata, to be able to substract this 1.
		decreaseKey(node, getMin()-1);
		node=0;
		//Make a delete min operation to remove the node from the heap.
		deleteMin();
	}
	
	
	//Returns a pointer to the node that follows the indices. Each index represents the an index of a list of children.
	//The first index is the index of the root, the second is the child of the root at the first index, etc.
	FH_node*	getNode(int* indices, int len)
	{
		FH_node* node(mBegin);
		
		for(register int cnt=0; cnt<len; cnt++)
		{
			//Only the first time we don't go to the child.
			if(cnt > 0) node=node->mDown;
			for(register int cnt2=0; cnt2<indices[cnt]; cnt2++)
				node=node->mRight;
		}
		return node;
	}
	
	
	//Given a pointer to a list of children, this method displays the trees starting at each node in a nested fashion, 
	//using the "<" and ">" as delimitators for the lists of children.
	void	displayChildren(FH_node* node, Tsize numChildren) const
	{
		cout<<" < ";
		for(register Tsize cnt=0; cnt<numChildren; cnt++)
		{
			cout<<node->mData.mKey;
			
			if(node->mData.mChildren>0)
				displayChildren(node->mDown, node->mData.mChildren);
			
			if(cnt<numChildren-1)
				cout<<", ";
			
			node=node->mRight;
		}
		cout<<" >";
	}
	
	
	//Displays the heap in a nested fashion, using the "<" and ">" as delimitators for the lists of children.
	void	display()	const
	{
		cout<<"Roots: "<<mRootSize<<".   Nodes: "<<mSize<<"\n";
		displayChildren(mBegin, mRootSize);
	}
	
	
	//Frees all memory used by the heap and sets members to default values.
	void	destroy()
	{
		while(mSize>0)
		{
			//We can applay a better technich like using the routine of 'deleteMin' but without the merging of the roots of equal degree.
			deleteMin(); 
		}
	}
	
	
	//Default constructor.
	Tfibonacciheap()
	{
		mSize=0;
		mRootSize=0;
		mBegin=0;
	}
	
	
	//The destructor.
	~Tfibonacciheap()
	{
		destroy();
	}
	
	
	//MEMBERS
    //The number of nodes in the heaps.
    Tsize       mSize;
	//The nomber of roots in the root set.
	Tsize		mRootSize;
    //The pointer to the start of the heap.
    FH_node*	mBegin;
};


//Overloading the print (<<) operator, to display the data in the array in the form:
// < 4, 5 < 9, 8 < 43, 30 < 56 > >, 6 < 7 > > >
//Where the outer most <> encloses the root set (keys) and each inner <>, to the right of a key, encloses its children.
template<typename Tsize, typename Tdata>
std::ostream& operator<<(std::ostream& os, const Tfibonacciheap<Tsize,Tdata>& FH)
{
	FH.printList();
	return os;
}

//********************************************************************************
//********************************************************************************

#endif
