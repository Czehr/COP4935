// identityTranslator clone
#include <rose.h>
using namespace std;

class visitorTraversal : public AstSimpleProcessing
{
  protected:
    virtual void visit(SgNode* n);
  private:
      SgNode* newClassNode;
  public:
      SgNode* getNewNode(){
        return newClassNode;
      }
};

void visitorTraversal::visit(SgNode* node)
{
  //if (node->variantT() == <insert Node Type Here>) {
  //    cout << "Found Node Type: " << <insert Node type Here> << endl;
  //}
}


int main (int argc, char** argv)
{
    // Build the AST used by ROSE
    SgProject* project = frontend(argc, argv);

    // Run internal consistency tests on AST
    AstTests::runAllTests(project);

    // Insert your own manipulations of the AST here...
    
	visitorTraversal myVisitor;
	myVisitor.traverseInputFiles(project, preorder);


  SgNode* newClassNode;

  Rose_STL_Container<SGNode*> classList2;
  classList2 = NodeQuery::querySubTree (project, V_SgClassDefinition);
  for(Rose_STL_Container<SgNode*>::iterator i = classList2.begin(); i != classList2.end(); i++){
    SgClassDefinition* ClassDefinition = isSgClassDefinition(*i);
    if(ClassDefinition->class_name() == "NodeInfo")
      newClassNode = myVisitor.getNewNode();
  }


	Rose_STL_Container<SgNode*> classList;
       classList = NodeQuery::querySubTree (project, V_SgClassDefinition);
       for(Rose_STL_Container<SgNode*>::iterator i = classList.begin(); i != classList.end(); i++){
         SgClassDefinition* ClassDefinition = isSgClassDefinition(*i);
         if(ClassDefinition->class_name() == "Node")
           ClassDefinition->append_member(isSgClassDeclaration(newClassNode));
       }



    // Generate source code from AST and invoke your
    // desired backend compiler
    return backend(project);
