#include <poincare/subtraction.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <poincare/addition.h>
#include <poincare/multiplication.h>
#include <poincare/opposite.h>
#include <poincare/rational.h>
#include <assert.h>

namespace Poincare {

int SubtractionNode::polynomialDegree(Context * context, const char * symbolName) const {
  int degree = 0;
  for (ExpressionNode * e : children()) {
    int d = e->polynomialDegree(context, symbolName);
    if (d < 0) {
      return -1;
    }
    degree = d > degree ? d : degree;
  }
  return degree;
}

// Private

bool SubtractionNode::childAtIndexNeedsUserParentheses(const Expression & child, int childIndex) const {
  if (childIndex == 0) {
    // First operand of a subtraction never requires parentheses
    return false;
  }
  if (child.isNumber() && static_cast<const Number &>(child).sign() == Sign::Negative) {
    return true;
  }
  if (child.type() == Type::Conjugate) {
    return childAtIndexNeedsUserParentheses(child.childAtIndex(0), childIndex);
  }
  Type types[] = {Type::Subtraction, Type::Opposite, Type::Addition};
  return child.isOfType(types, 3);
}

Layout SubtractionNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Infix(Subtraction(this), floatDisplayMode, numberOfSignificantDigits, "-");
}

int SubtractionNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
    return SerializationHelper::Infix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, "-");
}

Expression SubtractionNode::shallowReduce(ReductionContext reductionContext) {
  return Subtraction(this).shallowReduce(reductionContext);
}

Expression Subtraction::shallowReduce(ExpressionNode::ReductionContext reductionContext) {
  Expression e = Expression::defaultShallowReduce();
  if (e.isUndefined()) {
    return e;
  }

  if (reductionContext.hasFools() && numberOfChildren() == 2) {
    Expression e1 = childAtIndex(0);
    Expression e2 = childAtIndex(1);
    if (e1.type() == ExpressionNode::Type::Rational && e2.type() == ExpressionNode::Type::Rational) {
      Rational r1 = static_cast<Rational &>(e1);
      Rational r2 = static_cast<Rational &>(e2);
      if (r1.isInteger() && r2.isInteger() && r1.signedIntegerNumerator().isEight() && r2.signedIntegerNumerator().isTwo()) {
        // 8 - 2 = 5 :)
        Expression truth = Rational::Builder(5);
        replaceWithInPlace(truth);
        return truth.shallowReduce(reductionContext);
      }
    }
  }

  Expression m = Multiplication::Builder(Rational::Builder(-1), childAtIndex(1));
  Addition a = Addition::Builder(childAtIndex(0), m);
  m = m.shallowReduce(reductionContext);
  replaceWithInPlace(a);
  return a.shallowReduce(reductionContext);
}

}
