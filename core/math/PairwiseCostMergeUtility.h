#pragma once
#include <vector>
#include <unordered_set>

template<typename ObjectT, typename CostT>
class PairwiseCostMergeUtility {
  private:
    struct Pair {
      ObjectT *left, *right;
      CostT cost;
      Pair(ObjectT* left, ObjectT* right, CostT cost) : left(left), right(right), cost(cost) { }
    };
    std::vector<Pair> pairs_;
    std::unordered_set<ObjectT*> object_set_;

    inline bool is_object_merged(ObjectT* obj) {
      return object_set_.find(obj) == object_set_.end();
    }

    inline void initialize_object_set(std::vector<ObjectT*>& objects) {
      for(auto o : objects)
        object_set_.insert(o);
    }

    inline void initialize_pairs(std::vector<ObjectT*>& objects, auto& pair_cost) {
      for(int i = 0; i < objects.size(); i++) {
        auto lobject = objects[i];
        for(int j = i + 1; j < objects.size(); j++) {
          auto robject = objects[j];
          pairs_.push_back(Pair(lobject, robject, pair_cost(lobject, robject)));
        }
      }
    }
  public:
    std::vector<ObjectT*> merge_items(
        std::vector<ObjectT*> objects, // The objects that we want to merge
        int count, // The desired count after merging (-1 for threshold)
        CostT threshold, // The maximum pairwise cost to allow for merging
        std::function<ObjectT*(const ObjectT*,const ObjectT*)> merge_pair, // The method for merging two items together
        std::function<CostT(const ObjectT*,const ObjectT*)> pair_cost // The method for determining pairwise merge cost
      ) 
    {

      // We are already below the desired post-merge size, so no merging is necessary
      if(static_cast<int>(objects.size()) <= count) {
        return objects;
      }

      initialize_object_set(objects);
      initialize_pairs(objects, pair_cost);
      
      while(static_cast<int>(object_set_.size()) > count) {
        // Quit if there are no objects to merge
        if(object_set_.size() <= 1) break;

        // Sort pairs by decreasing cost
        std::sort(pairs_.begin(), pairs_.end(),
          [](Pair left, Pair right) {
            // Return true if left comes before right
            return left.cost > right.cost;
        });

        // Try to merge the closest pair
        Pair best = pairs_.back();
        if(best.cost > threshold) break;

        // If the objects have already been merged into a new pair,
        // they are no longer valid and should be removed.
        // TODO: This shouldn't ever happen though, verify
        while(is_object_merged(best.left) || is_object_merged(best.right)) {
          pairs_.pop_back();
          best = pairs_.back();
        }

        // Merge the best pair and delete the old ones
        auto merged = merge_pair(best.left, best.right);
        object_set_.erase(best.left);
        object_set_.erase(best.right);
        delete best.left;
        delete best.right;

        // Create a new pair list from old pairs that don't contain the
        // left/right objects that were just merged, as well as a new
        // pair for the object that was just merged and each pre-existing
        // object.
        std::vector<Pair> pcopy;
        for(auto pair : pairs_)
          if(pair.left != best.left && pair.right != best.left && pair.left != best.right && pair.right != best.right)
            pcopy.push_back(pair);
        pairs_ = pcopy;
        for(auto object : object_set_)
          // TODO: Don't create pairs for left/best or right/best
          pairs_.push_back(Pair(object, merged, pair_cost(object, merged)));
        object_set_.insert(merged);
      }

      // Fill up a vector with the results and reset member variables for the next run
      auto v = std::vector<ObjectT*>(object_set_.begin(), object_set_.end());
      object_set_.clear();
      pairs_.clear();
      return v;
    }
};
