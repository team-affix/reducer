#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <cstddef>
#include <memory>
#include <ostream>
#include <vector>

namespace lambda
{

struct expr
{
    using global_map = std::vector<std::unique_ptr<expr>>;
    virtual ~expr() = default;
    virtual bool equals(const std::unique_ptr<expr>&) const = 0;
    virtual void print(std::ostream& a_ostream) const = 0;
    virtual std::unique_ptr<expr> lift(size_t a_cutoff,
                                       size_t a_lift_amount) const = 0;
    virtual std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const = 0;
    virtual std::unique_ptr<expr> reduce(size_t a_depth,
                                         const global_map& a_globals) const = 0;
    std::unique_ptr<expr> clone() const;
    expr();
    expr(const expr& other) = delete;
    expr& operator=(const expr& other) = delete;
};

struct local : expr
{
    virtual ~local() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_cutoff,
                               size_t a_lift_amount) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(size_t a_depth,
                                 const global_map& a_globals) const override;
    size_t m_index;

  private:
    local(size_t a_index);
    friend std::unique_ptr<expr> l(size_t a_index);
};

struct global : expr
{
    virtual ~global() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_cutoff,
                               size_t a_lift_amount) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(size_t a_depth,
                                 const global_map& a_globals) const override;
    size_t m_index;

  private:
    global(size_t a_index);
    friend std::unique_ptr<expr> g(size_t a_index);
};

struct func : expr
{
    virtual ~func() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_cutoff,
                               size_t a_lift_amount) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(size_t a_depth,
                                 const global_map& a_globals) const override;
    std::unique_ptr<expr> m_body;

  private:
    func(const std::unique_ptr<expr>& a_body);
    friend std::unique_ptr<expr> f(const std::unique_ptr<expr>& a_body);
};

struct app : expr
{
    virtual ~app() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_cutoff,
                               size_t a_lift_amount) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(size_t a_depth,
                                 const global_map& a_globals) const override;
    std::unique_ptr<expr> m_func;
    std::unique_ptr<expr> m_arg;

  private:
    app(const std::unique_ptr<expr>& a_func,
        const std::unique_ptr<expr>& a_arg);
    friend std::unique_ptr<expr> a(const std::unique_ptr<expr>& a_func,
                                   const std::unique_ptr<expr>& a_arg);
};

// factory functions
std::unique_ptr<expr> l(size_t a_index);
std::unique_ptr<expr> g(size_t a_index);
std::unique_ptr<expr> f(const std::unique_ptr<expr>& a_body);
std::unique_ptr<expr> a(const std::unique_ptr<expr>& a_func,
                        const std::unique_ptr<expr>& a_arg);

} // namespace lambda

#endif
