#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"

using namespace std;
using namespace clang;
using namespace oclint;

class YBQRequestBlockCaptureRequestRule : public AbstractASTVisitorRule<YBQRequestBlockCaptureRequestRule>
{
public:
    virtual const string name() const override
    {
        return "YBQ request block capture request";
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
    
    /* Visit BinaryOperator */
    bool VisitBinaryOperator(BinaryOperator *node)
    {
        //获取二元运算符
        StringRef opcodeStr = node->getOpcodeStr();
        
        //判断是否为=运算符
        if (opcodeStr.str() != "=") {
            return true;
        }
        
        //判断是否为block属性赋值
        Expr *lhs = node->getLHS();
        ObjCPropertyRefExpr *lhsPropertyRefExpr = dyn_cast_or_null<ObjCPropertyRefExpr>(lhs);
        Expr *rhs = node->getRHS();
        OpaqueValueExpr *rhsOpaqueValueExpr = dyn_cast_or_null<OpaqueValueExpr>(rhs);
        if (rhsOpaqueValueExpr == NULL) {
            return true;
        }
        Expr *rhsSourceExpr = rhsOpaqueValueExpr->getSourceExpr();
        BlockExpr *rhsBlockExpr = dyn_cast_or_null<BlockExpr>(rhsSourceExpr);
        if (lhsPropertyRefExpr == NULL ||
            rhsBlockExpr == NULL) {
            return true;
        }
        
        //获取被赋值的属性名称
        ObjCPropertyDecl *lhsPropertyDecl = lhsPropertyRefExpr->getExplicitProperty();
        string lhsPropertyName = lhsPropertyDecl->getName();
        
        //判断被赋值属性是否符合要求
        if (lhsPropertyName != "requestSuccessBlock" &&
            lhsPropertyName != "requestFailBlock" &&
            lhsPropertyName != "requestExceptionBlock") {
            return true;
        }
        
        //获取Request的类型
        string lhsRequestType = lhsPropertyRefExpr->getBase()->getType().getAsString();
        //判断是否强引用request
        BlockDecl *rhsBlockDecl = rhsBlockExpr->getBlockDecl();
        for (BlockDecl::capture_const_iterator iterator = rhsBlockDecl->capture_begin(); iterator != rhsBlockDecl->capture_end(); iterator++) {
            VarDecl *varDecl = iterator->getVariable();
            string captureVarType = varDecl->getType().getAsString();
            if (captureVarType.find(lhsRequestType) != string::npos &&
                captureVarType.find("__strong") != string::npos) {
                addViolation(node, this, "request的回调中引用request会引起内存泄漏");
                break;
            }
        }
        
        return true;
    }
    
private:
    string BlockExprString(BlockExpr *node)
    {
        clang::SourceManager *sourceManager = &_carrier->getSourceManager();
        SourceLocation startLocation = node->getLocStart();
        SourceLocation endLocation = node->getLocEnd();
        if (sourceManager->isMacroBodyExpansion(startLocation) ||
            sourceManager->isInSystemMacro(startLocation) ||
            sourceManager->isMacroArgExpansion(startLocation)) {
            return "";
        }
        
        SourceLocation startSpellingLoc = startLocation, endSpellingLoc = endLocation;
        if (!startLocation.isFileID()) {
            startSpellingLoc = sourceManager->getSpellingLoc(startLocation);
            endSpellingLoc = sourceManager->getSpellingLoc(endLocation);
        }
        int length = sourceManager->getFileOffset(endSpellingLoc) -  sourceManager->getFileOffset(startSpellingLoc) + 1;
        if (length > 0) {
            string blockStatementString = StringRef(sourceManager->getCharacterData(startLocation), length).str();
            return blockStatementString;
        }
        
        return "";
    }
    
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
};

static RuleSet rules(new YBQRequestBlockCaptureRequestRule());
