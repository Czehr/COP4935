// identityTranslator clone
#include <rose.h>
using namespace std;

class visitorTraversal : public AstSimpleProcessing
{
  protected:
    virtual void visit(SgNode* n);
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

/* 
Insert #include "filename" or #include <filename> (system header) onto the global scope 
of a source file, add to be the last #include .. by default among existing headers, Or 
as the first header. Recommended for use. 
*/
//PreprocessingInfo * SageInterface::insertHeader (SgSourceFile *source_file, const std::string &header_file_name, bool isSystemHeader, bool asLastHeader)

	SgSourceFile *sourceFile;
	const SgFilePtrList& fileList = project->get_fileList();
	SgFilePtrList::const_iterator file = fileList.begin();
	sourceFile = isSgSourceFile(*file);
	const std::string &headerFileName = "LFTT.h";
	PreprocessingInfo::RelativePositionType position = PreprocessingInfo::before;
	bool isSystemHeader = false;

	SageInterface::insertHeader (sourceFile, headerFileName, isSystemHeader, position);

	


    // Generate source code from AST and invoke your
    // desired backend compiler
    return backend(project);
}
