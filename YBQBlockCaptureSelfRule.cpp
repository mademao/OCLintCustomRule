#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"
#include <regex>
#include <string>

using namespace std;
using namespace clang;
using namespace oclint;

class YBQBlockCaptureSelfRule : public AbstractASTVisitorRule<YBQBlockCaptureSelfRule>
{
public:
    virtual const string name() const override
    {
        return "YBQ block capture self";
    }
    
    virtual int priority() const override
    {
        return 2;
    }
    
    virtual const string category() const override
    {
        return "云表情自定义";
    }
    
    virtual void setUp() override {}
    virtual void tearDown() override {}
    
    /* Visit BlockDecl */
    bool VisitBlockDecl(BlockDecl *node)
    {
        //获取是否捕获了self
        bool captureSelf = false;
        string selfType = "";
        for (BlockDecl::capture_const_iterator iterator = node->capture_begin(); iterator != node->capture_end(); iterator++) {
            VarDecl *varDecl = iterator->getVariable();
            //捕获的实例变量一般为name:self,type:ViewController1 *const __strong
            //捕获的类变量一般为name:self,type:const Class
            if (varDecl &&
                varDecl->getName() == "self" &&
                varDecl->getType().getAsString().find("__strong") != string::npos) {
                captureSelf = true;
                selfType = varDecl->getType().getAsString();
                string_replace(selfType, "__strong", "");
                break;
            }
        }
        
        if (captureSelf == false) { //未捕获self，则无需进行判断
            return true;
        }
        
        //获取是否捕获了weakSelf
        bool captureWeakSelf = false;
        string captureWeakSelfName = "";
        for (BlockDecl::capture_const_iterator iterator = node->capture_begin(); iterator != node->capture_end(); iterator++) {
            VarDecl *varDecl = iterator->getVariable();
            if (varDecl) {
                string captureType = varDecl->getType().getAsString();
                string_replace(captureType, "__weak", "");
                if (captureType == selfType) {
                    captureWeakSelf = true;
                    captureWeakSelfName = varDecl->getName();
                    break;
                }
            }
        }
        
        //既捕获了self，又捕获了weakSelf，则建议使用weakSelf
        if (captureWeakSelf == true) {
            addViolation(node, this, "Block中请统一使用" + captureWeakSelfName);
            return true;
        }
        
        //捕获了self，检查Block中是否使用self.
        if (BlockDeclHasUseString(node, "self") == true ||
            BlockDeclHasUseString(node, "super") == true) {
            //如果使用了self，无论是直接使用还是访问属性，无论访问属性是否全部使用self.还是部分使用，此时都引用了self，无需提示
            //如果使用了super，虽然super是编译器指令，但是也会捕获self
        } else {
            //没有使用self却捕获了self，说明直接访问了实例变量
            addViolation(node, this, "Block中访问实例变量请显式写出'self.'");
        }
        
        return true;
    }
    
private:
    
    ///替换字符串中字符
    void string_replace( std::string &strBig, const std::string &strsrc, const std::string &strdst)
    {
        std::string::size_type pos = 0;
        std::string::size_type srclen = strsrc.size();
        std::string::size_type dstlen = strdst.size();
        
        while( (pos=strBig.find(strsrc, pos)) != std::string::npos )
        {
            strBig.replace( pos, srclen, strdst );
            pos += dstlen;
        }
    }
    
    ///检测Block中是否含有字符串
    bool BlockDeclHasUseString(BlockDecl *node, string str)
    {
        clang::SourceManager *sourceManager = &_carrier->getSourceManager();
        SourceLocation startLocation = node->getLocStart();
        SourceLocation endLocation = node->getLocEnd();
        if (sourceManager->isMacroBodyExpansion(startLocation) ||
            sourceManager->isInSystemMacro(startLocation) ||
            sourceManager->isMacroArgExpansion(startLocation)) {
            return true;
        }
        
        SourceLocation startSpellingLoc = startLocation, endSpellingLoc = endLocation;
        if (!startLocation.isFileID()) {
            startSpellingLoc = sourceManager->getSpellingLoc(startLocation);
            endSpellingLoc = sourceManager->getSpellingLoc(endLocation);
        }
        int length = sourceManager->getFileOffset(endSpellingLoc) -  sourceManager->getFileOffset(startSpellingLoc);
        if (length > 0) {
            string blockStatementString = StringRef(sourceManager->getCharacterData(startLocation) + 1, length).str();
            return blockStatementString.find(str) != string::npos;
        }
        
        return false;
    }
    
};

static RuleSet rules(new YBQBlockCaptureSelfRule());
