#ifndef HXWK_TYPES_H
#define HXWK_TYPES_H

#include <memory>
#include <type_traits>

class Type {
  public:
    enum class TypeKind { Simple, Void, Bool, Int32, Double, StrLit, Function };

    virtual ~Type() = default;

    Type(TypeKind kind) : kind{kind} {};
    TypeKind getKind() const { return kind; };

    virtual bool operator==(const Type &rhs) const {
        return kind == rhs.getKind();
    };
    bool operator!=(const Type &rhs) { return !(*this == rhs); };

  private:
    const TypeKind kind;
};

class SimpleType : public virtual Type {
  public:
    SimpleType() : Type{TypeKind::Simple} {};

    static bool classof(const Type *type) {
        return type->getKind() >= TypeKind::Simple
               && type->getKind() <= TypeKind::StrLit;
    };
};

class VoidType : public SimpleType {
  public:
    VoidType() : Type{TypeKind::Void} {};

    static bool classof(const Type *type) {
        return type->getKind() == TypeKind::Void;
    };
};

class BoolType : public SimpleType {
  public:
    BoolType() : Type{TypeKind::Bool} {};

    static bool classof(const Type *type) {
        return type->getKind() == TypeKind::Bool;
    };
};

class Int32Type : public SimpleType {
  public:
    Int32Type() : Type{TypeKind::Int32} {};

    static bool classof(const Type *type) {
        return type->getKind() == TypeKind::Int32;
    };
};

class DoubleType : public SimpleType {
  public:
    DoubleType() : Type{TypeKind::Double} {};

    static bool classof(const Type *type) {
        return type->getKind() == TypeKind::Double;
    };
};

class StrLitType : public SimpleType {
  public:
    StrLitType() : Type{TypeKind::StrLit} {};

    static bool classof(const Type *type) {
        return type->getKind() == TypeKind::StrLit;
    };
};

class FunctionType : public Type {
  public:
    FunctionType(std::size_t n_args, std::shared_ptr<Type> ret_type)
            : Type{TypeKind::Function},
              n_args{n_args},
              ret_type{std::move(ret_type)} {};

    std::size_t get_n_args() const { return n_args; };
    const std::shared_ptr<Type> &get_ret_type() const { return ret_type; };

    static bool classof(const Type *type) {
        return type->getKind() == TypeKind::Function;
    };

    bool operator==(const Type &rhs) const override {
        std::printf("kek, der shit\n");

        const FunctionType &rhs_fn = *static_cast<const FunctionType *>(&rhs);

        return Type::operator==(rhs) && ret_type == rhs_fn.ret_type
               && n_args == rhs_fn.n_args;
    };

  private:
    std::size_t n_args;
    std::shared_ptr<Type> ret_type;
};

#endif
