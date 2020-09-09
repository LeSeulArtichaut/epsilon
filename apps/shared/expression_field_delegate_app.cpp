#include "expression_field_delegate_app.h"
#include <escher.h>
#include <apps/i18n.h>
#include <poincare/expression.h>
#include <poincare/based_integer.h>

using namespace Poincare;

namespace Shared {

ExpressionFieldDelegateApp::ExpressionFieldDelegateApp(Snapshot * snapshot, ViewController * rootViewController) :
  TextFieldDelegateApp(snapshot, rootViewController),
  LayoutFieldDelegate()
{
}

bool ExpressionFieldDelegateApp::layoutFieldShouldFinishEditing(LayoutField * layoutField, Ion::Events::Event event) {
  return isFinishingEvent(event);
}

bool ExpressionFieldDelegateApp::layoutFieldDidReceiveEvent(LayoutField * layoutField, Ion::Events::Event event) {
  if (layoutField->isEditing() && layoutField->shouldFinishEditing(event)) {
    if (!layoutField->hasText()) {
      displayWarning(I18n::Message::SyntaxError);
      return true;
    }
    /* An acceptable layout has to be parsable and serialized in a fixed-size
     * buffer. We check all that here. */
    /* Step 1: Simple layout serialisation. Resulting texts can be parsed but
     * not displayed, like:
     * - 2a
     * - log_{2}(x) */
    constexpr int bufferSize = TextField::maxBufferSize();
    char buffer[bufferSize];
    int length = layoutField->layout().serializeForParsing(buffer, bufferSize);
    if (length >= bufferSize-1) {
      /* If the buffer is totally full, it is VERY likely that writeTextInBuffer
       * escaped before printing utterly the expression. */
      displayWarning(I18n::Message::SyntaxError);
      return true;
    }
    // Step 2: Parsing
    Poincare::Expression e = Poincare::Expression::Parse(buffer, layoutField->context());
    if (e.isUninitialized()) {
      // Unparsable expression
      displayWarning(I18n::Message::SyntaxError);
      return true;
    }
    if (Poincare::Preferences::sharedPreferences()->hasFools()) { // :)
      using namespace Poincare;

      switch (e.type()) {
        case ExpressionNode::Type::Multiplication:
          if (shouldLearnTables(static_cast<Multiplication &>(e))) {
            displayWarning(I18n::Message::LearnYourTables);
            return true;
          }
          break;
        case ExpressionNode::Type::Addition:
          if (playsChildrenGames(static_cast<Addition &>(e))) {
            displayWarning(I18n::Message::ChildGame);
            return true;
          }
          break;
        default:
          break;
      }
    }
    /* Step 3: Expression serialization. Tesulting texts are parseable and
     * displayable, like:
     * - 2*a
     * - log(x,2) */
    length = e.serialize(buffer, bufferSize, Poincare::Preferences::sharedPreferences()->displayMode());
    if (length >= bufferSize-1) {
      // Same comment as before
      displayWarning(I18n::Message::SyntaxError);
      return true;
    }
    if (!isAcceptableExpression(e)) {
      displayWarning(I18n::Message::SyntaxError);
      return true;
    }
  }
  if (fieldDidReceiveEvent(layoutField, layoutField, event)) {
    return true;
  }
  return false;
}

bool shouldLearnTables(Poincare::Multiplication m) {
  using namespace Poincare;
  if (m.numberOfChildren() == 2) {
    Expression e1 = m.childAtIndex(0);
    Expression e2 = m.childAtIndex(1);
    if (e1.type() == ExpressionNode::Type::BasedInteger && e2.type() == ExpressionNode::Type::BasedInteger) {
      BasedInteger i1 = static_cast<BasedInteger &>(e1);
      BasedInteger i2 = static_cast<BasedInteger &>(e2);
      if (i1.integer().isLowerThan(11) && i2.integer().isLowerThan(11)) {
        return true;
      }
    }
  }
  return false;
}

bool playsChildrenGames(Poincare::Addition a) {
  using namespace Poincare;
  if (a.numberOfChildren() == 3) {
    Expression e1 = a.childAtIndex(0);
    Expression e2 = a.childAtIndex(1);
    Expression e3 = a.childAtIndex(2);
    if (e1.type() == ExpressionNode::Type::BasedInteger && e2.type() == ExpressionNode::Type::BasedInteger && e3.type() == ExpressionNode::Type::BasedInteger) {
      Integer i1 = static_cast<BasedInteger &>(e1).integer();
      Integer i2 = static_cast<BasedInteger &>(e2).integer();
      Integer i3 = static_cast<BasedInteger &>(e3).integer();
      if (i1.isOne() && i2.isTwo() && i3.isThree()) {
        return true;
      }
    }
  }
  return false;
}

}
