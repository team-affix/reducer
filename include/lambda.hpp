#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <cstddef>
#include <memory>
#include <ostream>

namespace lambda
{

struct expr
{
    virtual ~expr() = default;
    virtual bool equals(const std::unique_ptr<expr>&) const = 0;
    virtual void print(std::ostream& a_ostream) const = 0;
    virtual std::unique_ptr<expr> lift(size_t a_lift_amount,
                                       size_t a_cutoff) const = 0;
    virtual std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const = 0;
    virtual std::unique_ptr<expr> reduce_one_step(size_t a_depth) const = 0;
    std::unique_ptr<expr> clone() const;
    std::unique_ptr<expr> normalize() const;
    expr();
    expr(const expr& other) = delete;
    expr& operator=(const expr& other) = delete;
};

struct var : expr
{
    virtual ~var() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_lift_amount,
                               size_t a_cutoff) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce_one_step(size_t a_depth) const override;
    size_t m_index;

  private:
    var(size_t a_index);
    friend std::unique_ptr<expr> v(size_t a_index);
};

struct func : expr
{
    virtual ~func() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_lift_amount,
                               size_t a_cutoff) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce_one_step(size_t a_depth) const override;
    std::unique_ptr<expr> m_body;

  private:
    func(std::unique_ptr<expr>&& a_body);
    friend std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body);
};

struct app : expr
{
    virtual ~app() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_lift_amount,
                               size_t a_cutoff) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce_one_step(size_t a_depth) const override;
    std::unique_ptr<expr> m_func;
    std::unique_ptr<expr> m_arg;

  private:
    app(std::unique_ptr<expr>&& a_func, std::unique_ptr<expr>&& a_arg);
    friend std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_func,
                                   std::unique_ptr<expr>&& a_arg);
};

// factory functions
std::unique_ptr<expr> v(size_t a_index);
std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body);
std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_func,
                        std::unique_ptr<expr>&& a_arg);

} // namespace lambda

#endif
