#include <gtest/gtest.h>
#include "../include/CreditScorer.h"

class CreditScorerTest : public ::testing::Test {
protected:
    CreditScorer scorer;
};

TEST_F(CreditScorerTest, UnderageApplicantRejected) {
    CreditApplicant applicant;
    applicant.age = 17;
    applicant.monthlyIncome = 600000;
    applicant.currentBalance = 200000;
    applicant.loanAmount = 100000;
    applicant.loanTermMonths = 12;

    CreditAssessment result = scorer.evaluate(applicant);
    EXPECT_FALSE(result.isApproved);
}

TEST_F(CreditScorerTest, LowRiskApplicantApproved) {
    CreditApplicant applicant;
    applicant.age = 30;
    applicant.monthlyIncome = 800000;
    applicant.currentBalance = 500000;
    applicant.loanAmount = 300000;
    applicant.loanTermMonths = 12;

    CreditAssessment result = scorer.evaluate(applicant);
    EXPECT_TRUE(result.isApproved);
    EXPECT_GE(result.score, 670);
}

TEST_F(CreditScorerTest, HighDebtRatioRejected) {
    CreditApplicant applicant;
    applicant.age = 25;
    applicant.monthlyIncome = 80000;
    applicant.currentBalance = 5000;
    applicant.loanAmount = 5000000;
    applicant.loanTermMonths = 6;

    CreditAssessment result = scorer.evaluate(applicant);
    EXPECT_FALSE(result.isApproved);
}
