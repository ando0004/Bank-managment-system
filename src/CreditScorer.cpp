#include "CreditScorer.h"
#include <cmath>
#include <algorithm>

CreditScorer::CreditScorer() {
    weights = {0.041852, 1.382914, 0.791245, -1.214088, 0.389102};
    intercept = 0.081429;

    scalerMean = {42.853000, 448120.450000, 598412.100000, 1195430.000000, 36.120000};
    scalerScale = {12.651200, 149302.180000, 348910.400000, 698230.500000, 16.241000};
}

double CreditScorer::sigmoid(double z) const {
    return 1.0 / (1.0 + std::exp(-z));
}

CreditAssessment CreditScorer::evaluate(const CreditApplicant& applicant) const {
    CreditAssessment assessment;
    if (applicant.age < 18) {
        assessment.score = 300;
        assessment.isApproved = false;
        assessment.riskLevel = "HIGH";
        return assessment;
    }
    std::vector<double> features = {
        static_cast<double>(applicant.age),
        applicant.monthlyIncome,
        applicant.currentBalance,
        applicant.loanAmount,
        static_cast<double>(applicant.loanTermMonths)
    };

    double z = intercept;
    for (size_t i = 0; i < features.size(); ++i) {
        double normalized = (features[i] - scalerMean[i]) / scalerScale[i];
        z += normalized * weights[i];
    }

    double prob = sigmoid(z);
    int creditScore = static_cast<int>(300 + prob * 550);
    creditScore = std::max(300, std::min(850, creditScore));

    CreditAssessment res;
    res.score = creditScore;
    res.probability = prob;

    if (creditScore >= 670) {
        res.isApproved = true;
        res.riskLevel = "LOW RISK (Approved)";
    } else if (creditScore >= 580) {
        res.isApproved = true;
        res.riskLevel = "MEDIUM RISK (Review Needed)";
    } else {
        res.isApproved = false;
        res.riskLevel = "HIGH RISK (Rejected)";
    }

    return res;
}
