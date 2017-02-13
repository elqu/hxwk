#define ABSTR_ACCEPT(visitor_type)                                            \
    virtual void accept(visitor_type &visitor) const = 0

#define ACCEPT(visitor_type)                                                  \
    void accept(visitor_type &visitor) const override { visitor.visit(*this); }

#define ABSTR_VISIT(visitable_type)                                           \
    virtual void visit(const visitable_type &visitable) = 0

#define VISIT(visitable_type)                                                 \
    void visit(const visitable_type &visitable) override
