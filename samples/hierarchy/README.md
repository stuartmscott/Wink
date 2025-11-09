# Hierarchy

Demonstrates different tree shapes of Hierarchical State Machines;

## Empty

A tree with no nodes.

## Leaf

A tree with a single leaf node.

```
  Leaf
```

## Simple

A tree with a parent and two child leaf nodes.

```
     Parent
     /    \
    /      \
  Leaf1   Leaf2
```

## Bigger

A tree with three levels of nodes.

```
     Parent
     /    \
    /      \
  Leaf1    Child1
           /   \
          /     \
        Leaf2  Leaf3
```

## Forrest

A set of multiple trees ranging in size and shape.

```
     Parent1            Leaf1            Parent2                 Parent3
     /    \                              /    \                     |
    /      \                            /      \                    |
  Leaf2    Child2                    Child3    Child4             Leaf3
           /   \                      /          \
          /     \                    /            \
       Leaf4   Leaf5              Leaf6         Leaf7
```
