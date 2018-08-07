#ifndef PATTERNS__CONFIGURATION_H
#define PATTERNS__CONFIGURATION_H


template<typename Config, typename Left, typename Right>
class PatternNode;

template <typename Entity, typename Projection>
struct Configuration {
};

template <typename Node>
struct set_projection;


template <typename Node>
struct set_entity_t;

template <typename Entity, typename Projection>
struct set_projection<Configuration<Entity, Projection>> {

  template<typename NewProjection>
  struct to {
    typedef Configuration<Entity, NewProjection> type;
  };
};

template <typename Entity, typename Projection>
struct set_entity_t<Configuration<Entity, Projection>> {

  template<typename NewEntityT>
  struct to {
    typedef Configuration<NewEntityT, Projection> type;
  };
};

template<typename ConfigurationT, typename Left, typename Right>
struct set_projection<PatternNode<ConfigurationT, Left, Right>> {
  template <typename NewProjection>
  struct to {
    typedef PatternNode<
        typename set_projection<ConfigurationT>::template to<
            NewProjection>::type,
        Left,
        Right>
        type;
  };
};

template<typename ConfigurationT, typename Left, typename Right>
struct set_entity_t<PatternNode<ConfigurationT, Left, Right>> {
  template <typename NewEntityT>
  struct to {
    typedef PatternNode<
        typename set_entity_t<ConfigurationT>::template to<NewEntityT>::type,
        Left,
        Right>
        type;
  };
};

#endif
