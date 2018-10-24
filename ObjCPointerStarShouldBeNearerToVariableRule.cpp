#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"

using namespace std;
using namespace clang;
using namespace oclint;

class ObjCPointerStarShouldBeNearerToVariableRule : public AbstractASTVisitorRule<ObjCPointerStarShouldBeNearerToVariableRule>
{
    
public:
    virtual const string name() const override {
        return "测试name";
    }
    virtual int priority() const override {
        return 3;
    }
    virtual const string category() const override {
        return "测试category";
    }
    virtual unsigned int supportedLanguages() const override{
        return LANG_OBJC;
    }
    
//    bool VisitObjCPropertyDecl(ObjCPropertyDecl *decl){
//        string type = decl->getType().getAsString();
//        ObjCPropertyDecl::SetterKind setKid = decl->getSetterKind();
//        string name = decl->getNameAsString();
//        string::iterator itor = name.begin();
//        if(*itor >= 'A' && *itor <= 'Z'){
//            addViolation(decl, this, "Property "+name+" start with uppercase letter");
//        }
//        return true;
//    }
    
    /* Visit ObjCPropertyImplDecl */
     bool VisitObjCPropertyImplDecl(ObjCPropertyImplDecl *node)
     {
         ObjCPropertyDecl *decl = node->getPropertyDecl();
         ObjCPropertyDecl::SetterKind setKid = decl->getSetterKind();
         string name = decl->getNameAsString();
         string::iterator itor = name.begin();
         if(*itor >= 'A' && *itor <= 'Z'){
             addViolation(decl, this, "Property "+name+" start with uppercase letter");
         }
         return true;
     }
     

};

static RuleSet rules(new ObjCPointerStarShouldBeNearerToVariableRule());
