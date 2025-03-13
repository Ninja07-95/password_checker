#include <iostream>
#include <string>
#include <map>
#include <regex>

class PasswordStrengthChecker {
private:
    // Constantes pour les seuils de force
    const int MIN_LENGTH = 8;
    const int GOOD_LENGTH = 12;
    const int STRONG_LENGTH = 16;
    
    // Coefficients pour calculer le score
    const double LENGTH_WEIGHT = 0.4;
    const double COMPLEXITY_WEIGHT = 0.6;
    
    // Attributs
    std::string password;
    std::map<std::string, bool> criteria;
    double score;
    std::string strength;

public:
    PasswordStrengthChecker(const std::string& pwd) : password(pwd), score(0.0), strength("") {
        criteria["length"] = false;
        criteria["uppercase"] = false;
        criteria["lowercase"] = false;
        criteria["digit"] = false;
        criteria["special"] = false;
        criteria["no_sequence"] = false;
    }
    
    void analyzePassword() {
        // Vérification de la longueur
        criteria["length"] = (password.length() >= MIN_LENGTH);
        
        // Vérification des caractères
        criteria["uppercase"] = std::regex_search(password, std::regex("[A-Z]"));
        criteria["lowercase"] = std::regex_search(password, std::regex("[a-z]"));
        criteria["digit"] = std::regex_search(password, std::regex("[0-9]"));
        criteria["special"] = std::regex_search(password, std::regex("[^a-zA-Z0-9]"));
        
        // Vérification des séquences communes
        criteria["no_sequence"] = !hasCommonSequences();
        
        // Calcul du score
        calculateScore();
        
        // Détermination de la force
        determineStrength();
    }
    
    bool hasCommonSequences() {
        std::vector<std::string> commonSequences = {
            "123456", "password", "qwerty", "abc123", "123123", "admin",
            "12345678", "1234567890", "abcdef", "azerty"
        };
        
        for (const auto& seq : commonSequences) {
            if (password.find(seq) != std::string::npos) {
                return true;
            }
        }
        
        // Vérification des caractères répétés
        for (size_t i = 0; i < password.length() - 2; i++) {
            if (password[i] == password[i + 1] && password[i] == password[i + 2]) {
                return true;
            }
        }
        
        return false;
    }
    
    void calculateScore() {
        // Score basé sur la longueur (40%)
        double lengthScore = 0.0;
        if (password.length() >= STRONG_LENGTH) {
            lengthScore = 1.0;
        } else if (password.length() >= GOOD_LENGTH) {
            lengthScore = 0.7;
        } else if (password.length() >= MIN_LENGTH) {
            lengthScore = 0.3;
        }
        
        // Score basé sur la complexité (60%)
        double complexityScore = 0.0;
        int criteriaCount = 0;
        for (const auto& criterion : criteria) {
            if (criterion.first != "length" && criterion.second) {
                criteriaCount++;
            }
        }
        complexityScore = criteriaCount / 5.0;  // 5 critères hors longueur
        
        // Score final
        score = (lengthScore * LENGTH_WEIGHT) + (complexityScore * COMPLEXITY_WEIGHT);
    }
    
    void determineStrength() {
        if (score >= 0.8) {
            strength = "Très fort";
        } else if (score >= 0.6) {
            strength = "Fort";
        } else if (score >= 0.4) {
            strength = "Moyen";
        } else if (score >= 0.2) {
            strength = "Faible";
        } else {
            strength = "Très faible";
        }
    }
    
    void displayResults() {
        std::cout << "\n=== Analyse du mot de passe ===" << std::endl;
        std::cout << "Longueur: " << password.length() << " caractères" << std::endl;
        std::cout << "\nCritères satisfaits:" << std::endl;
        std::cout << "- Longueur minimale: " << (criteria["length"] ? "Oui" : "Non") << std::endl;
        std::cout << "- Lettres majuscules: " << (criteria["uppercase"] ? "Oui" : "Non") << std::endl;
        std::cout << "- Lettres minuscules: " << (criteria["lowercase"] ? "Oui" : "Non") << std::endl;
        std::cout << "- Chiffres: " << (criteria["digit"] ? "Oui" : "Non") << std::endl;
        std::cout << "- Caractères spéciaux: " << (criteria["special"] ? "Oui" : "Non") << std::endl;
        std::cout << "- Absence de séquences communes: " << (criteria["no_sequence"] ? "Oui" : "Non") << std::endl;
        
        std::cout << "\nScore: " << score * 100 << "/100" << std::endl;
        std::cout << "Force: " << strength << std::endl;
        
        std::cout << "\nRecommandations:" << std::endl;
        if (!criteria["length"]) {
            std::cout << "- Utilisez un mot de passe d'au moins " << MIN_LENGTH << " caractères" << std::endl;
        }
        if (!criteria["uppercase"]) {
            std::cout << "- Ajoutez au moins une lettre majuscule" << std::endl;
        }
        if (!criteria["lowercase"]) {
            std::cout << "- Ajoutez au moins une lettre minuscule" << std::endl;
        }
        if (!criteria["digit"]) {
            std::cout << "- Ajoutez au moins un chiffre" << std::endl;
        }
        if (!criteria["special"]) {
            std::cout << "- Ajoutez au moins un caractère spécial" << std::endl;
        }
        if (!criteria["no_sequence"]) {
            std::cout << "- Évitez les séquences communes ou les caractères répétés" << std::endl;
        }
    }
};

int main() {
    std::string password;
    char choice;
    
    do {
        std::cout << "\n=== Vérificateur de Force de Mot de Passe ===" << std::endl;
        std::cout << "Entrez un mot de passe: ";
        std::getline(std::cin, password);
        
        PasswordStrengthChecker checker(password);
        checker.analyzePassword();
        checker.displayResults();
        
        std::cout << "\nSouhaitez-vous vérifier un autre mot de passe? (o/n): ";
        std::cin >> choice;
        std::cin.ignore();  // Pour consommer le caractère de nouvelle ligne
        
    } while (choice == 'o' || choice == 'O');
    
    std::cout << "Merci d'avoir utilisé le vérificateur de force de mot de passe!" << std::endl;
    
    return 0;
}
