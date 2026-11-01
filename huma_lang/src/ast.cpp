#include "huma/ast.hpp"

namespace huma {

void LiteralExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void IdentifierExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void BinaryExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void UnaryExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void CallExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ListExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void DictExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void IndexExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void MemberExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }

void VarDeclStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ChangeStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ShowStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void IfStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void RepeatTimesStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void WhileStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ForEachStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void FuncDeclStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ReturnStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ClassDeclStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void TryCatchStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ImportStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void BreakStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ContinueStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ExprStmt::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void Program::accept(ASTVisitor& visitor) { visitor.visit(*this); }

} // namespace huma
