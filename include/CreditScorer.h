#pragma once
#include <string>
#include <vector>

struct CreditApplicant {
    int age;
    double monthlyIncome;
    double currentBalance;
    double loanAmount;
    int loanTermMonths;
};

struct CreditAssessment {
    int score;
    double probability;
    bool isApproved;
    std::string riskLevel;
};

class CreditScorer {
private:
    std::vector<double> weights;
    double intercept;
    std::vector<double> scalerMean;
    std::vector<double> scalerScale;

    double sigmoid(double z) const;

public:
    CreditScorer();
    CreditAssessment evaluate(const CreditApplicant& applicant) const;
};
