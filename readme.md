# FormulaZ

## (1)
样例 'a + b / c - 1'  
转换为token 'a', '+', 'b', '/', 'c', '-', '1'  
通过token自顶向下检查是否有syntax error 

## (2)
把token转换为expr_node，生成表达式树  
```
        [ - ]
       /     \
      /       \
    [ + ]    [ 1 ]
   /     \
  /       \
a        [ / ]
        /     \
       /       \
    [ b ]      [ c ]
```
## (3)
把表达式树转为渲染树  
```
    [___root___]
    /  /  |  \  \
   /  /   |   \  \
[a] [+]  [b]  [/] [1]
              / \
             /   \
            [b]  [c]
```
## (4)
渲染表达式树