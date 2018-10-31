#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"

using namespace std;
using namespace clang;
using namespace oclint;

class YBQPropertySetterKidRule : public AbstractASTVisitorRule<YBQPropertySetterKidRule>
{
public:
    virtual const string name() const override
    {
        return "属性对应的修饰词设置不正确";
    }
    
    virtual int priority() const override
    {
        return 1;
    }
    
    virtual const string category() const override
    {
        return "云表情自定义";
    }
    
    virtual unsigned int supportedLanguages() const override{
        return LANG_OBJC;
    }
    
    bool VisitObjCPropertyDecl(ObjCPropertyDecl *node)
    {
        ObjCPropertyDecl::PropertyAttributeKind attributeKind = node->getPropertyAttributes();
        string type = node->getType().getAsString();
        string name = node->getNameAsString();
        bool isBlock = node->getType()->isBlockPointerType();
        
        
        if (isBlock == true) {
            if ((attributeKind & ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_strong) == ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_strong) {
                addViolation(node, this, "Block属性 "+name+" 使用copy修饰比使用strong修饰更能表达内存操作方式");
            } else if ((attributeKind & ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_copy) != ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_copy) {
                addViolation(node, this, "Block属性 "+name+" 应当使用copy修饰");
            }
        } else if (is_begin_with(type.c_str(), "NSString") ||
                   is_begin_with(type.c_str(), "NSArray") ||
                   is_begin_with(type.c_str(), "NSDictionary") ||
                   is_begin_with(type.c_str(), "NSSet") ||
                   is_begin_with(type.c_str(), "NSData") ||
                   is_begin_with(type.c_str(), "NSURLRequest") ) {
            if ((attributeKind & ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_copy) != ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_copy) {
                addViolation(node, this, type+"属性 "+name+" 应当使用copy修饰");
            }
        } else if (is_begin_with(type.c_str(), "NSMutableString") ||
                   is_begin_with(type.c_str(), "NSMutableArray") ||
                   is_begin_with(type.c_str(), "NSMutableDictionary") ||
                   is_begin_with(type.c_str(), "NSMutableSet") ||
                   is_begin_with(type.c_str(), "NSMutableData") ||
                   is_begin_with(type.c_str(), "NSMutableURLRequest") ) {
            if ((attributeKind & ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_strong) != ObjCPropertyDecl::PropertyAttributeKind::OBJC_PR_strong) {
                addViolation(node, this, type+"属性 "+name+" 应当使用strong修饰");
            }
        }
        
        return true;
    }
    
private:
    bool is_begin_with(const char * str1, string str2)
    {
        char *ch = (char *)str2.c_str();
        if(str1 == NULL || ch == NULL)
            return false;
        int len1 = strlen(str1);
        int len2 = strlen(ch);
        if((len1 < len2) ||  (len1 == 0 || len2 == 0))
            return false;
        char *p = ch;
        int i = 0;
        while(*p != '\0')
        {
            if(*p != str1[i])
                return 0;
            p++;
            i++;
        }
        return true;
    }
};

static RuleSet rules(new YBQPropertySetterKidRule());
