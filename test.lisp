((assert (eq "hi" [?104 ?105]))

 [1 2 3 4 5 6 7 8 9 10]

 (defun factorial (n)
   "Calculate the factorial of integer N.

Let's test some \"escape sequences\".\r\nHello\tWorld!"
   (if (or (= n 0) (= n 1)) 1
       (* (factorial (- n 2)) (factorial (- n 1)))))

 (print (factorial 10)))
