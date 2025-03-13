#include <cmath>
#include <set>
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>

class PasswordStrengthChecker {
private:
    // Constantes pour les seuils de force
    const int MIN_LENGTH = 8;
    const int GOOD_LENGTH = 12;
    const int STRONG_LENGTH = 16;
    
    // Coefficients pour calculer le score
    const double LENGTH_WEIGHT = 0.35;
    const double COMPLEXITY_WEIGHT = 0.45;
    const double ENTROPY_WEIGHT = 0.20;
    
    // Attributs
    std::string password;
    std::map<std::string, bool> criteria;
    double score;
    std::string strength;
    
    // Liste des mots de passe courants (à charger depuis un fichier)
    std::vector<std::string> commonPasswords;
    
    // Temps estimé pour cracker le mot de passe
    std::string crackTime;

public:
    PasswordStrengthChecker(const std::string& pwd) : password(pwd), score(0.0), strength("") {
        criteria["length"] = false;
        criteria["uppercase"] = false;
        criteria["lowercase"] = false;
        criteria["digit"] = false;
        criteria["special"] = false;
        criteria["no_sequence"] = false;
        criteria["no_common"] = false;
        criteria["unique_chars"] = false;
        
        // Chargement des mots de passe courants
        loadCommonPasswords();
    }
    
    void loadCommonPasswords() {
        // Liste intégrée de mots de passe courants (pourrait être chargée depuis un fichier(rockme par exemple, mais il prendra du temps, donc je dois implémenter des threads))
        commonPasswords = {
            "123456", "password", "12345678", "qwerty", "123456789", "12345", "1234", 
            "111111", "1234567", "dragon", "123123", "baseball", "abc123", "football", 
            "monkey", "letmein", "696969", "shadow", "master", "666666", "qwertyuiop", 
            "123321", "mustang", "1234567890", "michael", "654321", "superman", "1qaz2wsx", 
            "7777777", "121212", "000000", "qazwsx", "azerty", "1111", "iloveyou", "admin",
            "welcome", "sunshine", "chocolate", "password1"
        };
        
	// Importer un fichier des mots de passes courrants

        // Tentative de chargement depuis un fichier si disponible
        std::ifstream file("common_passwords.txt");
        if (file.is_open()) {
            commonPasswords.clear();
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty()) {
                    commonPasswords.push_back(line);
                }
            }
            file.close();
        }
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
        
        // Vérification si le mot de passe est dans la liste des mots de passe courants
        criteria["no_common"] = !isCommonPassword();
        
        // Vérification de la diversité des caractères
        criteria["unique_chars"] = hasGoodCharacterDiversity();
        
        // Calcul du score
        calculateScore();
        
        // Détermination de la force
        determineStrength();
        
        // Estimation du temps pour cracker
        estimateCrackTime();
    }
    
    bool hasCommonSequences() {
        // Séquences de clavier
        std::vector<std::string> keyboardSequences = {
            "qwerty", "azerty", "qwertz", "asdfgh", "zxcvbn", "1234567890"
        };
        
        for (const auto& seq : keyboardSequences) {
            for (size_t i = 0; i <= seq.length() - 3; i++) {
                std::string subseq = seq.substr(i, 3);
                if (password.find(subseq) != std::string::npos || 
                    password.find(std::string(subseq.rbegin(), subseq.rend())) != std::string::npos) {
                    return true;
                }
            }
        }
        
        // Vérification des séquences numériques ou alphabétiques
        for (size_t i = 0; i < password.length() - 2; i++) {
            if ((password[i+1] == password[i] + 1 && password[i+2] == password[i] + 2) ||  // séquence croissante
                (password[i+1] == password[i] - 1 && password[i+2] == password[i] - 2)) {  // séquence décroissante
                return true;
            }
        }
        
        // Vérification des caractères répétés
        for (size_t i = 0; i < password.length() - 2; i++) {
            if (password[i] == password[i+1] && password[i] == password[i+2]) {
                return true;
            }
        }
        
        return false;
    }
    
    bool isCommonPassword() {
        // Vérifier si le mot de passe est dans la liste des mots de passe courants
        std::string lowercasePwd = password;
        std::transform(lowercasePwd.begin(), lowercasePwd.end(), lowercasePwd.begin(), ::tolower);
        
        for (const auto& commonPwd : commonPasswords) {
            if (lowercasePwd == commonPwd) {
                return true;
            }
        }
        
        // Vérifier les variantes simples (ajout de chiffres à la fin, etc.)
        std::string basePassword = lowercasePwd;
        // Enlever les chiffres à la fin
        while (!basePassword.empty() && std::isdigit(basePassword.back())) {
            basePassword.pop_back();
        }
        
        if (!basePassword.empty() && basePassword != lowercasePwd) {
            for (const auto& commonPwd : commonPasswords) {
                if (basePassword == commonPwd) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    bool hasGoodCharacterDiversity() {
        // Calculer le nombre de caractères uniques
        std::set<char> uniqueChars(password.begin(), password.end());
        double uniqueRatio = static_cast<double>(uniqueChars.size()) / password.length();
        
        // Au moins 50% des caractères devraient être uniques
        return uniqueRatio >= 0.5;
    }
    
    double calculateEntropy() {
        // Calculer l'entropie du mot de passe (bits)
        double charsetSize = 0;
        if (criteria["lowercase"]) charsetSize += 26;
        if (criteria["uppercase"]) charsetSize += 26;
        if (criteria["digit"]) charsetSize += 10;
        if (criteria["special"]) charsetSize += 33;
        
        if (charsetSize == 0) charsetSize = 26; // Au moins les minuscules
        
        return password.length() * log2(charsetSize);
    }
    
    void calculateScore() {
        // Score basé sur la longueur (35%)
        double lengthScore = 0.0;
        if (password.length() >= STRONG_LENGTH) {
            lengthScore = 1.0;
        } else if (password.length() >= GOOD_LENGTH) {
            lengthScore = 0.7;
        } else if (password.length() >= MIN_LENGTH) {
            lengthScore = 0.3;
        }
        
        // Score basé sur la complexité (45%)
        double complexityScore = 0.0;
        int criteriaCount = 0;
        int totalCriteria = 0;
        
        for (const auto& criterion : criteria) {
            if (criterion.first != "length") {
                totalCriteria++;
                if (criterion.second) {
                    criteriaCount++;
                }
            }
        }
        
        complexityScore = static_cast<double>(criteriaCount) / totalCriteria;
        
        // Score basé sur l'entropie (20%)
        double entropy = calculateEntropy();
        double entropyScore = 0.0;
        
        if (entropy >= 70) entropyScore = 1.0;
        else if (entropy >= 60) entropyScore = 0.9;
        else if (entropy >= 50) entropyScore = 0.7;
        else if (entropy >= 40) entropyScore = 0.5;
        else if (entropy >= 30) entropyScore = 0.3;
        else entropyScore = 0.1;
        
        // Pénalités
        double penalties = 0.0;
        
        // Pénalité pour les mots de passe courants
        if (!criteria["no_common"]) {
            penalties += 0.3;
        }
        
        // Pénalité pour les séquences
        if (!criteria["no_sequence"]) {
            penalties += 0.2;
        }
        
        // Score final
        score = (lengthScore * LENGTH_WEIGHT) + 
                (complexityScore * COMPLEXITY_WEIGHT) + 
                (entropyScore * ENTROPY_WEIGHT);
        
        // Appliquer les pénalités
        score = std::max(0.0, score - penalties);
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
    
    void estimateCrackTime() {
        // Estimation grossière basée sur l'entropie
        double entropy = calculateEntropy();
        
        // Hypothèse: 10^9 tentatives par seconde (attaque brute force moderne)
        double secondsToCrack = pow(2, entropy) / 1000000000.0;
        
        if (secondsToCrack < 60) {
            crackTime = "Instantané";
        } else if (secondsToCrack < 3600) {
            crackTime = std::to_string(static_cast<int>(secondsToCrack / 60)) + " minutes";
        } else if (secondsToCrack < 86400) {
            crackTime = std::to_string(static_cast<int>(secondsToCrack / 3600)) + " heures";
        } else if (secondsToCrack < 31536000) {
            crackTime = std::to_string(static_cast<int>(secondsToCrack / 86400)) + " jours";
        } else if (secondsToCrack < 315360000) { // 10 ans
            crackTime = std::to_string(static_cast<int>(secondsToCrack / 31536000)) + " ans";
        } else if (secondsToCrack < 3153600000) { // 100 ans
            crackTime = "Plusieurs décennies";
        } else if (secondsToCrack < 31536000000) { // 1000 ans
            crackTime = "Plusieurs siècles";
        } else {
            crackTime = "Pratiquement incassable";
        }
    }
    
    std::string generateStrongPassword() {
        const std::string lowercase = "abcdefghijklmnopqrstuvwxyz";
        const std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const std::string digits = "0123456789";
        const std::string special = "!@#$%^&*()-_=+[]{}|;:,.<>?";
        
        std::string chars = lowercase + uppercase + digits + special;
        
        // Longueur aléatoire entre 16 et 20
        std::srand(std::time(0));
        int length = 16 + std::rand() % 5;
        
        std::string password;
        
        // S'assurer d'avoir au moins un caractère de chaque type
        password += lowercase[std::rand() % lowercase.size()];
        password += uppercase[std::rand() % uppercase.size()];
        password += digits[std::rand() % digits.size()];
        password += special[std::rand() % special.size()];
        
        // Remplir le reste aléatoirement
        for (int i = 4; i < length; i++) {
            password += chars[std::rand() % chars.size()];
        }
        
        // Mélanger le mot de passe
        std::random_shuffle(password.begin(), password.end());
        
        return password;
    }
    void displayResults() {
        // Effet de chargement pour l'analyse
        std::cout << "Analyse en cours";
        for (int i = 0; i < 5; i++) {
            std::cout << ".";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::cout << std::endl;

        // Affichage des résultats
        std::cout << "\n========== Analyse du mot de passe ==========\n" << std::endl;
        std::cout << "Longueur: " << password.length() << " caractères" << std::endl;

        // Affichage de l'entropie
        std::cout << "Entropie estimée: " << calculateEntropy() << " bits" << std::endl;

        std::cout << "\nCritères satisfaits:" << std::endl;
        std::cout << "✓ Longueur minimale (" << MIN_LENGTH << " caractères): "
                  << (criteria["length"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Lettres majuscules: "
                  << (criteria["uppercase"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Lettres minuscules: "
                  << (criteria["lowercase"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Chiffres: "
                  << (criteria["digit"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Caractères spéciaux: "
                  << (criteria["special"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Pas de séquences communes: "
                  << (criteria["no_sequence"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Pas de mot de passe courant: "
                  << (criteria["no_common"] ? "Oui" : "Non") << std::endl;
        std::cout << "✓ Diversité des caractères: "
                  << (criteria["unique_chars"] ? "Oui" : "Non") << std::endl;

        std::cout << "\nScore de force: " << score * 100 << "%" << std::endl;
        std::cout << "Force du mot de passe: " << strength << std::endl;
        std::cout << "Temps estimé pour cracker: " << crackTime << std::endl;

        std::cout << "\n============================================\n" << std::endl;
    }
};

int main() {
    std::string password;
    std::cout << "Entrez un mot de passe pour l'analyser : ";
    std::getline(std::cin, password);

    PasswordStrengthChecker checker(password);
    checker.analyzePassword();
    checker.displayResults();

    // Option : Générer un mot de passe fort
    char choice;
    std::cout << "Voulez-vous générer un mot de passe fort ? (o/n) : ";
    std::cin >> choice;

    if (choice == 'o' || choice == 'O') {
        std::string strongPassword = checker.generateStrongPassword();
        std::cout << "\nMot de passe fort généré : " << strongPassword << std::endl;

        // Analyser le mot de passe généré
        PasswordStrengthChecker strongChecker(strongPassword);
        strongChecker.analyzePassword();
        strongChecker.displayResults();
    }

    return 0;
}
