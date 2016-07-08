#include "tools.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>


using Category = std::string;
using Word = std::string;
using Text = std::vector<Word>;
using Id = long;

struct Document {
    Id id;
    std::unordered_map<Word, int /* freq */> words;
    size_t num_words;
};

class Model {
public:
    explicit Model(const rapidjson::Document& json) {
        for (auto docit = json.Begin(); docit != json.End(); ++docit) {
            auto& json_doc = *docit;
            Id id = json_doc["id"].GetInt();
            documents.emplace(id, make_document(id, json_doc));

            if (json_doc.HasMember("cuisine")) {
                cat_index[json_doc["cuisine"].GetString()].insert(id);
            }
        }
    }

public:
    std::unordered_map<Id, Document> documents;
    std::unordered_map<Category, std::unordered_set<Id>> cat_index;
    size_t vocabulary_size;

private:
    Document make_document(Id id, const rapidjson::Value& json) {
        Document doc;
        doc.id = id;
        auto& json_ingredients = json["ingredients"];
        for (auto it = json_ingredients.Begin();
                it != json_ingredients.End(); ++it)
        {
            for (const auto& word : split(it->GetString())) {
                doc.words[word] += 1;
                doc.num_words += 1;
            }
        }
        return doc;
    }
};


size_t tf(const Word& t, const Document& d) {
    auto pos = d.words.find(t);
    return pos != d.words.end() ? pos->second : 0;
}


double P_xic(
    const Model& model, const Word& xi, const Category& c, double alpha = 1.)
{
    static std::unordered_map<std::pair<Word, Category>, double> cache;
    auto cache_key = std::make_pair(xi, c);
    auto pos = cache.find(cache_key);
    if (pos != cache.end()) {
        return pos->second;
    }

    double nom = alpha;
    // double denom = alpha * model.vocabulary_size;
    // double denom = 1;

    for (const auto id : model.cat_index.at(c)) {
        const auto& d = model.documents.at(id);
        nom += tf(xi, d);
        // denom += d.num_words;
    }

    cache[cache_key] = nom;
    return nom;
    // return nom / denom;
}

double P_xc(const Model& model, const Text& x, const Category& c) {
    double prod = 1;
    for (const auto& xi : x) {
        prod *= P_xic(model, xi, c);
    }
    return prod;
}


Category classify(const Model& model, const Text& x) {
    Category cat;
    double max_prob;
    for (const auto& c : keys(model.cat_index)) {
        double p = P_xc(model, x, c);
        if (cat.empty() || max_prob < p) {
            max_prob = p;
            cat = c;
        }
    }
    return cat;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <train.json> <test.json>"
            << std::endl;
        return 1;
    }

    std::cerr << "Loading train data..." << std::endl;
    rapidjson::Document json;
    {
        std::ifstream is(argv[1]);
        rapidjson::IStreamWrapper isw(is);

        json.ParseStream(isw);
        assert(json.IsArray());
    }
    Model train_data(json);

    std::cerr << "Loading test data..." << std::endl;
    {
        std::ifstream is(argv[2]);
        rapidjson::IStreamWrapper isw(is);

        json.ParseStream(isw);
        assert(json.IsArray());
    }
    Model test_data(json);

    std::cerr << "Classifying..." << std::endl;
    size_t pos = 0;
    std::cout << "id,cuisine" << std::endl;
    for (const auto& doc : values(test_data.documents)) {
        auto words = keys(doc.words);
        Text x(words.begin(), words.end());
        std::cout << doc.id << "," << classify(train_data, x) << std::endl;

        // output progress
        size_t percent = test_data.documents.size() / 100;
        if (pos++ % percent == percent - 1) {
            std::cerr << "." << std::flush;
        }
    }
    std::cerr << std::endl;

    return 0;
}
