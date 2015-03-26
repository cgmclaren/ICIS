C******************************************************* modSparseMatrix
C  PURPOSE:
C  LIBRARY ROUTINES FOR SPARSE MATRIX COMPUTATIONS
C
C  Author: Douglas B. Clarkson, ScienceOps
C
C  Copyright (C) 2005 IRRI
C
C  This program is free software; you can redistribute it and/or
C  modify it under the terms of the GNU General Public License
C  as published by the Free Software Foundation; either version 2
C  of the License, or (at your option) any later version.
C
C  This program is distributed in the hope that it will be useful,
C  but WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C  GNU General Public License for more details.
C  You should have received a copy of the GNU General Public License
C  along with this program; if not, write to the Free Software
C  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
C
C 1.1 ---------------------------------------------------------------
C 09/09/11 
C Changed the ALLOCATE statement in function allocateElement of module
C modSparseMatrix to return the STATE of the allocate command which is
C positive when memory is unavailable. Otherwise the application exits
C abruptly. Passed the STATE to the calling subroutines which checks for
C a non zero state and executes an alternative RETURN if found so that 
C the application terminates in a controlled fashion.
C 1.0 ---------------------------------------------------------------
C 02/02/06 
      module modSparseMatrix
      implicit none
C Define the basic types
C******************************************************* cElement
C  PURPOSE:
C  An element in a sparse matrix
C
C  COMPONENTS:
C  dataVal   - double precision matrix element value
C  iRow      - the row number
C  iCol      - the column number
C  nextInRow - pointer to the next element in the row
C  nextInCol - pointer to the next element in the column
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 cElement
      type cElement
          private
          real(8) :: dataVal
          integer iRow
          integer iCol
          type(cElement), pointer :: nextInRow
          type(cElement), pointer :: nextInCol  
      end type cElement
C******************************************************* cPtrCElement
C  PURPOSE:
C  This type is required because FORTRAN90 does not allow arrays of pointers
C
C  COMPONENTS:
C  p - pointer to an element in a sparse matrix
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 cPtrCElement
      type cPtrCElement
          private
          type(cElement), pointer :: p
      end type cPtrCElement
C******************************************************* cMatrix
C  PURPOSE:
C  A sparse matrix
C
C  COMPONENTS:
C  nRow  - number of rows in the complete matrix
C  nCol  - number of columns in the complete matrix
C  firstInRow - pointer to the first element in each row
C  firstInCol - pointer to the first element in each column
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 cMatrix
      type cMatrix
          private
          integer(kind=4) :: nRow
          integer(kind=4) :: nCol
          type(cPtrCElement), pointer :: firstInRow(:)
          type(cPtrCElement), pointer :: firstInCol(:)
      end type cMatrix
C******************************************************* SparseMatrix
C  PURPOSE:
C  Module Declarations
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 SparseMatrix
      type(cPtrCElement), allocatable :: stackPtr(:)
      integer :: stackLength, stackNext
C  Define module subroutines and functions
      contains
C******************************************************* initializeStack
C  PURPOSE:
C  Initializes the stack of sparse matrix elements.
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 initializeStack
      subroutine initializeStack(n)
            integer, intent(in) :: n
C Local variables
            integer i
            allocate(stackPtr(n))
            stackLength = n
            stackNext = n
            do i = 1, n
              allocate(stackPtr(i)%p)
            end do
      end subroutine initializeStack
C******************************************************* deallocateStack
C  PURPOSE:
C  Deallocate the stack of sparse matrix elements.
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 deallocateStack
      subroutine deallocateStack()
C Local variables
      integer i
            do i = 1, stackLength
              deallocate(stackPtr(i)%p)
            end do
      deallocate(stackPtr)
      end subroutine deallocateStack
C******************************************************* allocateElement
C  PURPOSE:
C  Gets an element from the stack of sparse matrix elements.
C
C  ARGUMENTS:
C  k returns with 0 if space was successfully allocated
C  K>0 otherwise
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 allocateElement
      function allocateElement(k) result(ptr)
            type(cElement), pointer :: ptr
            integer, intent(out) :: k
C Local variables
            integer :: i, j
            k=0
            if (stackNext > 0) then
                 ptr => stackPtr(stackNext)%p
                 stackNext = stackNext - 1
            else
                 i = stackLength
                 j = stackNext
                 deallocate(stackPtr)
                 allocate(stackPtr(2*stackLength),STAT=k)
                 if(k.ne.0) return
                 stackNext = stackLength - 1
                 do i = 1, stackLength
                      allocate(stackPtr(i)%p,STAT=k)
                      if(k.ne.0) return             
                 end do
                 ptr => stackPtr(stackLength)%p
                 stackLength = 2*stackLength
            end if
      end function allocateElement
C******************************************************* deallocateElement
C  PURPOSE:
C  Adds the cElement pointed to by ptr back to the stack of
C  sparse matrix elements.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 deallocateElement
      subroutine deallocateElement(ptr)
            type(cElement), intent(in), target :: ptr
            integer :: i, j
            i = stackLength
            j = stackNext
            stackPtr(stackNext+1)%p => ptr
            stackNext = stackNext + 1
      end subroutine deallocateElement
C******************************************************* GetNCol_mf_int
C  PURPOSE:
C  Returns the number of columns of sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 GetNCol_mf_int
      PURE function GetNCol_mf_int(X) result(nCol_int)
          type(cMatrix), intent(in) :: X
          integer :: nCol_int
          nCol_int = X%nCol
      end function GetNCol_mf_int
C******************************************************* setNCol
C  PURPOSE:
C  Returns the number of columns of sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 02/11/04 setNCol
      subroutine setNCol(X, nCol)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: nCol
            X%nCol = nCol
      end subroutine setNCol
C******************************************************* GetNRow_mf_int
C  PURPOSE:
C  Returns the number of rows of sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 GetNRow_mf_int
      PURE function GetNRow_mf_int(X) result(nRow_int)
          type(cMatrix), intent(in) :: X
          integer :: nRow_int
          nRow_int = X%nRow
      end function GetNRow_mf_int
C******************************************************* nullifyPtrs
C  PURPOSE:
C  This routine is required because FORTRAN90 does not allow an
C  array of pointers.  It nullify an array of type(cPtrElement).
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 nullifyPtrs
      subroutine nullifyPtrs(n, x)
          integer, intent(in) :: n
          type(cPtrCElement), intent(inout) :: x(:)
C  Local variables
          integer i
          do i=1, n
              nullify(x(i)%p)
          end do
      end subroutine nullifyPtrs
C******************************************************* setDataVal
C  PURPOSE:
C  Sets the dataVal element of a cElement object.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 setDataVal
      subroutine setDataVal(ptr, value)
          type(cElement), pointer :: ptr
          real(8), intent(in) :: value
          ptr%dataVal = value
      end subroutine setDataVal
C******************************************************* getDataVal
C  PURPOSE:
C  Retrieves the dataVal element of a cElement object.
C
C  ARGUMENTS:
C  ptr - cElement object retrieved from a sparse matrix
C  value - the value from the dataVal component of ptr
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getDataVal
      function getDataVal(ptr) result(value)
        type(cElement), pointer :: ptr
        real(8) :: value
        value = ptr%dataVal
      end function getDataVal
C******************************************************* setSparseValue
C  PURPOSE:
C  Sets the i, j value for sparse matrix X.
C
C  ARGUMENTS:
C  X - Sparse matrix
C  i - row coordinate of desired element
C  j - column coordinate of desired element
C  value - the value for the i,j th element of X
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 setSparseValue
      subroutine setSparseValue(X, i, j, value,*)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: i
          integer, intent(in) :: j
          real(8), intent(in) :: value
C Local variables
          type(cElement), pointer :: ptr, ptrOld
          ptr => X%firstInRow(i)%p
          if (.not. ASSOCIATED(ptr)) then
              call addNewElement(X, i, j, value,*99)
          else if (ptr%iCol == j) then
              ptr%dataVal = value
          else
              do while (ASSOCIATED(ptr))
                  if (ptr%iCol == j) then
                      ptr%dataVal = value
                      EXIT
                  else if (ptr%iCol > j) then
                      call addNewElement(X, i, j, value,*99)
                      EXIT
                  else
                      ptr => ptr%nextInRow
                  end if
              end do
              call addNewElement(X, i, j, value,*99)
          end if
      RETURN
 99   WRITE(99,'('' setSparseValue''\)')
      RETURN 1
      end subroutine setSparseValue
C******************************************************* getSparseValue
C  PURPOSE:
C  Retrieves the ijth value (X(i,j)) from sparse matrix X.
C
C  ARGUMENTS:
C  X - Sparse matrix (input)
C  i - row coordinate of desired element
C  j - column coordinate of desired element
C
C  value - returns with the value of element i,j of X
C  If the value has not been set, value returns with zero.
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getSparseValue
      function getSparseValue(X, i, j) result(value)
          type(cMatrix), intent(in) :: X
          integer, intent(in) :: i
          integer, intent(in) :: j
          real(8) :: value
C  Local variables
          type(cElement), pointer :: ptr
          value = 0.0d0
          ptr => X%firstInRow(i)%p
          if (.not. ASSOCIATED(ptr)) then
              value = 0.0d0
          else if (ptr%iCol == j) then
              value = ptr%dataVal
          else
              value = 0.0d0
              ptr => ptr%nextInRow
              do while (ASSOCIATED(ptr))
                  if (ptr%iCol == j) then
                      value = ptr%dataVal
                      EXIT
                  else
                      ptr => ptr%nextInRow
                  end if
              end do
          end if
      end function getSparseValue
C******************************************************* setMatrix
C  PURPOSE:
C  Initialize the sparse matrix X to have nRow rows and nCol columns.
C
C  ARGUMENTS:
C  X - Sparse matrix (input)
C  nRow - the number of rows to be initialized in X
C  nCol - the number of columns to be initialized in X
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 setMatrix
      subroutine setMatrix(X, nRow, nCol)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: nRow, nCol
          X%nRow = nRow
          X%nCol = nCol
            if (nRow > 0 .and. nCol > 0) then
                 allocate(X%firstInRow(nRow))
                 allocate(X%firstInCol(nCol))
                 call nullifyPtrs(nRow, X%firstInRow)
                 call nullifyPtrs(nCol, X%firstInCol)
            end if
      end subroutine setMatrix
C******************************************************* setElement
C  PURPOSE:
C  Initialize the (iRow,iCol) element in sparse matrix x with value.
C
C  ARGUMENTS:
C  X - cElement object of a Sparse matrix
C  value - the value to be set for the  element
C  iRow - the row of the element to be set
C  iCol - the column of the element to be set
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 setElement
      subroutine setElement(x, value, iRow, iCol)
          type(cElement), intent(inout) :: x
          real(8), intent(in) :: value
          integer, intent(in) :: iRow
          integer, intent(in) :: iCol
          x%dataVal = value
          x%iRow = iRow
          x%iCol = iCol
          x%nextInRow => NULL()
          x%nextInCol => NULL()
      end subroutine setElement
C******************************************************* addSparseRow
C  PURPOSE:
C  Add a row to sparse matrix x.
C
C  REMARKS:
C  The row is assumed to be empty on entry.
C  nVal values are added to row iRow, each to the columns in array iCols
C  ptrLast is a vector of pointers to the last element in the column for each row
C  It is assumed that rows are added one at a time, beginning with row 1.
C  No check on this is performed.
C
C  ARGUMENTS:
C  x - sparce matrix
C  nVal - number of values to be added in row iRow
C  values - array of nval values to be set for row iRow
C  iRow - the row to contain the nVal values
C  iCol - array containing the column numbers for the nVal values
C         to be added to row iRow.
C  ptrLast - a vector of pointers to the last element in the column for each row
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 addSparseRow
      subroutine addSparseRow(x, nVal, values, iRow, iCols, ptrLast,*)
          type(cMatrix), intent(inout) :: x
          integer, intent(in) :: nVal
          real(8), intent(in) :: values(nVal)
          integer, intent(in) :: iRow
          integer, intent(in) :: iCols(nVal)
          type(cPtrCElement), intent(inout) :: ptrLast(:)
C  Local variables
          integer i,k
          type(cElement), pointer :: ptrElement
          type(cElement), pointer :: ptrRowLast
          if (ASSOCIATED(x%firstInRow(iRow)%p)) then
              call ERTN("Cannot add a row to the row of a matrix that al
     &ready has elements.",0,*99)
          end if
          ptrElement => allocateElement(k)
          if(k.ne.0) goto 99
          call setElement(ptrElement, values(1), iRow, iCols(1))
          x%firstInRow(iRow)%p => ptrElement
          if (.not. ASSOCIATED(x%firstInCol(iCols(1))%p)) then
              x%firstInCol(iCols(1))%p => ptrElement
          end if
          if (.not. ASSOCIATED(ptrLast(iCols(1))%p)) then
              ptrLast(iCols(1))%p => ptrElement
          else
              ptrLast(iCols(1))%p%nextInCol => ptrElement
              ptrLast(iCols(1))%p => ptrElement
          end if
          ptrRowLast => ptrElement
          do i=2,nVal
              ptrElement => allocateElement(k)
              if(k.ne.0) goto 99
              call setElement(ptrElement, values(i), iRow, iCols(i))
              ptrRowLast%nextInRow => ptrElement
              ptrRowLast => ptrElement
              if (.not. ASSOCIATED(x%firstInCol(iCols(i))%p)) then
                  x%firstInCol(iCols(i))%p => ptrElement
              end if
              if (.not. ASSOCIATED(ptrLast(iCols(i))%p)) then
                  ptrLast(iCols(i))%p => ptrElement
              else
                  ptrLast(iCols(i))%p%nextInCol => ptrElement
                  ptrLast(iCols(i))%p => ptrElement
              end if
          end do
      RETURN
 99   WRITE(99,'('' addSparseRow''\)')
      RETURN 1
      end subroutine addSparseRow
C******************************************************* addSparseCol
C  PURPOSE:
C  Add a column to sparse matrix x.
C
C  REMARKS:
C  The column is assumed to be empty on entry.
C  nVal values are added to column iCol, each to the rows in array iRows
C  ptrLast is a vector of pointers to the last element in the row for each column
C  It is assumed that columns are added one at a time, beginning with column 1.
C  No check on this is performed.
C
C  ARGUMENTS:
C  x - sparce matrix
C  nVal - number of values to be added in row iRow
C  values - array of nval values to be set for row iRow
C  iRows - array containing the rows to contain the nVal values in col iCol
C  iCol - the column for the nVal values
C  ptrLast - a vector of pointers to the last element in the row for each column
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 addSparseCol
      subroutine addSparseColumn(x,nVal,values,iRows,iCol,ptrLast,*)
          type(cMatrix), intent(inout) :: x
          integer, intent(in) :: nVal
          real(8), intent(in) :: values(nVal)
          integer, intent(in) :: iRows(nVal)
          integer, intent(in) :: iCol
          type(cPtrCElement), intent(inout) :: ptrLast(:)
C  Local variables
          integer i,k
          type(cElement), pointer :: ptrElement
          type(cElement), pointer :: ptrColLast
          if (ASSOCIATED(x%firstInCol(iCol)%p)) then
          call ERTN("Cannot add elements to the column of a matrix that
     & already has elements",0,*99)
          end if
          ptrElement => allocateElement(k)
          if(k.ne.0) goto 99
          call setElement(ptrElement, values(1), iRows(1), iCol)
          x%firstInCol(iCol)%p => ptrElement
          if (.not. ASSOCIATED(x%firstInRow(iRows(1))%p)) then
              x%firstInRow(iRows(1))%p => ptrElement
          end if
          if (.not. ASSOCIATED(ptrLast(iRows(1))%p)) then
              ptrLast(iRows(1))%p => ptrElement
          else
              ptrLast(iRows(1))%p%nextInRow => ptrElement
              ptrLast(iRows(1))%p => ptrElement
          end if
          ptrColLast => ptrElement
          do i=2,nVal
              ptrElement => allocateElement(k)
              if(k.ne.0) goto 99
              call setElement(ptrElement, values(i), iRows(i), iCol)
              ptrColLast%nextInCol => ptrElement
              ptrColLast => ptrElement
              if (.not. ASSOCIATED(x%firstInRow(iRows(i))%p)) then
                  x%firstInRow(iRows(i))%p => ptrElement
              end if
              if (.not. ASSOCIATED(ptrLast(iRows(i))%p)) then
                  ptrLast(iRows(i))%p => ptrElement
              else
                  ptrLast(iRows(i))%p%nextInRow => ptrElement
                  ptrLast(iRows(i))%p => ptrElement
              end if
          end do
      RETURN
 99   WRITE(99,'('' addSparseColumn''\)')
      RETURN 1
      end subroutine addSparseColumn
C******************************************************* clearSparseMatrix
C  PURPOSE:
C  Removes all elements in matrix X, and nullifys all pointers
C  The row an column dimensions remain unchanged.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 clearSparseMatrix
      subroutine clearSparseMatrix(X,*)
          type(cMatrix), intent(inout) :: X
C Local variable
          type(cElement), pointer :: tmp1, tmp2
          integer i
C deallocate the matrix elements by row, one row at a time
            if (X%nRow <= 0 .or. X%nCol <= 0) then
                 return
                 call ERTN("Cannot clear a matrix with no rows or " //
     &                      "columns in clearSparseMatrix.",0,*99)
            end if
          do i = 1, X%nRow
              tmp1 => X%firstInRow(i)%p
              do while (ASSOCIATED(tmp1))
                  tmp2 => tmp1%nextInRow
                  call deallocateElement(tmp1)
                  tmp1 => tmp2
              end do
              nullify(X%firstInRow(i)%p)
          end do
C nullify the column pointers
          call nullifyPtrs(X%nCol, X%firstInCol)
        RETURN
   99   WRITE(99,'('' clearSparseMatrix''\)')
        RETURN 1
      end subroutine clearSparseMatrix
C******************************************************* destroySparseMatrix
C  PURPOSE:
C  Return the allocated stack elements in a sparse matrix to the
C  stack. Deallocate allocated memory, and set nRow and nCol to 0.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 destroySparseMatrix
       subroutine destroySparseMatrix(X,*)
            type(cMatrix), intent(inout) :: X
C   remove all existing elements in X
            if (X%nRow > 0 .and. X%nCol > 0) then
                 call clearSparseMatrix(X,*99)
C   now reallocate the first in row pointers
                 if (ASSOCIATED(X%firstInRow)) then
                   deallocate(X%firstInRow)
                      nullify(X%firstInRow)
                 end if
              if (ASSOCIATED(X%firstInCol)) then
                    deallocate(X%firstInCol)
                      nullify(X%firstInCol)
                 end if
            end if
            X%nRow = 0
            X%nCol = 0
            return
   99          write(99,'('' destroySparseMatrix''\)')
            return 1
       end subroutine destroySparseMatrix
C******************************************************* reshapeSparseMatrix
C  PURPOSE:
C Reshape sparse matrix X to have nRow rows and nCol columns
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 reshapeSparseMatrix
      subroutine reshapeSparseMatrix(X, nRow, nCol,*)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: nRow
          integer, intent(in) :: nCol
          if (nRow < 0) then
              call ERTN("Cannot reshape a matrix to have a negative" //
     &                  " number of rows.",0,*99)
          end if
          if (nCol < 0) then
              call ERTN("Cannot reshape a matrix to have a negative" //
     &                  " number of columns.",0,*99)
          end if
C remove all existing elements in X
          call destroySparseMatrix(X,*99)
C now reallocate the first in row pointers
          if (nRow > 0 .and. nCol > 0) then
                 allocate(X%firstInRow(nRow))
                 allocate(X%firstInCol(nCol))
                 call nullifyPtrs(nRow, X%firstInRow)
                 call nullifyPtrs(nCol, X%firstInCol)
            end if
          X%nRow = nRow
          X%nCol = nCol
      RETURN
 99   WRITE(99,'('' reshapeSparseMatrix''\)')
      RETURN 1
      end subroutine reshapeSparseMatrix
C******************************************************* addRowsSparseMatrix
C  PURPOSE:
C  Add to the number of rows allowed in a sparse matrix.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 addRowsSparseMatrix
      subroutine addRowsSparseMatrix(X, nAdd,*)
            type(cMatrix), intent(inout) :: X
            integer, intent(in) :: nAdd
C  Local variables
            integer :: i
            type(cPtrCElement) :: firstInRow(X%nRow)
            if (nAdd <= 0) then
                 call ERTN("argument nAdd must be positive in" //
     &                      " addRowsSparseMatrix.",0,*99)
            end if
C Deal with the row pointers
            if (X%nRow > 0) then
                 do i = 1, X%nRow
                      firstInRow(i)%p => X%firstInRow(i)%p
                 end do
                 deallocate(X%firstInRow)
                 allocate(X%firstInRow(X%nRow+nAdd))
                 call nullifyPtrs(X%nRow+nAdd, X%firstInRow)
                 do i = 1, X%nRow
                      X%firstInRow(i)%p => firstInRow(i)%p
                 end do
            end if
            X%nRow = X%nRow + nAdd
        RETURN
   99   WRITE(99,'('' addRowsSparseMatrix''\)')
        RETURN 1
      end subroutine addRowsSparseMatrix
C******************************************************* multiplySparseMatrix
C  PURPOSE:
C Multiply sparse matrix X times sparse matrix Y to yield
C sparse matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseMatrix
      subroutine multiplySparseMatrix(X, Y, Z,*)
          type(cMatrix), intent(in) :: X, Y
          type(cMatrix), intent(inout) :: Z
C  Local variables
          real(8)   values(Y%nCol), sum
          integer i, j, m, iCols(Y%nCol)
          type(cElement), pointer :: ptrX, ptrY
          type(cPtrCElement) :: ptrLast(Y%nCol)
          call reshapeSparseMatrix(Z, X%nRow, Y%nCol,*99)
          call nullifyPtrs(Y%nCol, ptrLast)
C   Multiple each row of X (index i) times each column of Y (index j)
          do i = 1, X%nRow
              if (ASSOCIATED(X%firstInRow(i)%p)) then
C   if the row does not have any elements, then the result is null
                  m = 0
                  do j = 1, Y%nCol
                      if (ASSOCIATED(Y%firstInCol(j)%p)) then
C   if the column does not have any elements, then the result is null
                          ptrY => Y%firstInCol(j)%p
                          ptrX => X%firstInRow(i)%p
                          sum = 0.0d0
                          do
                              if (ptrX%iCol == ptrY%iRow) then
                                  sum = sum + ptrX%dataVal*ptrY%dataVal
                                  ptrX => ptrX%nextInRow
                                  if (.not. ASSOCIATED(ptrX)) exit
                                  ptrY => ptrY%nextInCol
                                  if (.not. ASSOCIATED(ptrY)) exit
                              else if (ptrX%iCol > ptrY%iRow) then
                                  ptrY => ptrY%nextInCol
                                  if (.not. ASSOCIATED(ptrY)) exit
                              else
                                  ptrX => ptrX%nextInRow
                                  if (.not. ASSOCIATED(ptrX)) exit
                              end if
                          end do
                          if (sum /= 0.0d0) then
                              m = m + 1
                              iCols(m) = j
                              values(m) = sum
                          end if
                      end if
                  end do
      if (m > 0) call addSparseRow(Z, m, values, i, iCols, ptrLast,*99)
              end if
          end do
      RETURN
  99  WRITE(99,'('' multiplySparseMatrix''\)')
      RETURN 1
      end subroutine multiplySparseMatrix
C******************************************************* ddotColSparse
C  PURPOSE:
C  Compute the inner product of two columns in sparse matrices.
C  The columns may be in different matrices.
C  No check on conformability of the two matrices is made.
C  ptrXIn and ptrYIn point to the first element in each column
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 ddotColSparse
      function ddotColSparse(ptrXIn, ptrYIn) result(sum)
          type(cElement), pointer :: ptrXIn, ptrYIn
          real(8) :: sum
          type(cElement), pointer :: ptrX, ptrY
          ptrX => ptrXIn
          ptrY => ptrYIn
          sum = 0.0d0
          do while (ASSOCIATED(ptrX) .and. ASSOCIATED(ptrY))
              if (ptrX%iRow == ptrY%iRow) then
                  sum = sum + ptrX%dataVal*ptrY%dataVal
                  ptrX => ptrX%nextInCol
                  ptrY => ptrY%nextInCol
              else if (ptrX%iRow > ptrY%iRow) then
                  ptrY => ptrY%nextInCol
              else
                  ptrX => ptrX%nextInCol
              end if
          end do
      end function ddotColSparse
C******************************************************* ddotRowSparse
C  PURPOSE:
C  Compute the inner product of two rows in sparse matrices.
C  The rows may be in different matrices.
C  No check on conformability of the two matrices is made.
C  ptrXIn and ptrYIn point to the first element in each row
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 ddotRowSparse
       function ddotRowSparse(ptrXIn, ptrYIn) result(sum)
          type(cElement), pointer :: ptrXIn, ptrYIn
          real(8) :: sum
          type(cElement), pointer :: ptrX, ptrY
          ptrX => ptrXIn
          ptrY => ptrYIn
          sum = 0.0d0
          do while (ASSOCIATED(ptrX) .and. ASSOCIATED(ptrY))
              if (ptrX%iCol == ptrY%iCol) then
                  sum = sum + ptrX%dataVal*ptrY%dataVal
                  ptrX => ptrX%nextInRow
                  ptrY => ptrY%nextInRow
              else if (ptrX%iCol > ptrY%iCol) then
                  ptrY => ptrY%nextInRow
              else
                  ptrX => ptrX%nextInRow
              end if
          end do
      end function ddotRowSparse
C******************************************************* ddotRowColSparse
C  PURPOSE:
C  Compute the inner product of a row with a column in sparse
C  matrices.  The columns may be in different matrices.
C  No check on conformability of the two matrices is made.
C  ptrXIn and ptrYIn point to the first element in the row and
C  column, respectively.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 ddotRowColSparse
      function ddotRowColSparse(ptrXIn, ptrYIn, offset) result(sum)
          type(cElement), pointer :: ptrXIn, ptrYIn
            integer, intent(in) :: offset
          real(8) :: sum
          type(cElement), pointer :: ptrX, ptrY
          ptrX => ptrXIn
          ptrY => ptrYIn
          sum = 0.0d0
          do while (ASSOCIATED(ptrX) .and. ASSOCIATED(ptrY))
              if (ptrX%iCol == ptrY%iRow+offset) then
                  sum = sum + ptrX%dataVal*ptrY%dataVal
                  ptrX => ptrX%nextInRow
                  ptrY => ptrY%nextInCol
              else if (ptrX%iCol > ptrY%iRow+offset) then
                  ptrY => ptrY%nextInCol
              else
                  ptrX => ptrX%nextInRow
              end if
          end do
      end function ddotRowColSparse
C******************************************************* ddotColSparseDense
C  PURPOSE:
C  Compute the inner product of columns in a sparse matrix
C  with a dense vector.  No check on conformability is made.
C  ptrXIn points to the first element in the column.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 ddotColSparseDense
       function ddotColSparseDense(ptrXIn, nRow, Y) result(sum)
          type(cElement), pointer :: ptrXIn
            integer, intent(in) :: nRow
            real(8), intent(in) :: Y(nRow)
          real(8) :: sum
          type(cElement), pointer :: ptrX
          ptrX => ptrXIn
          sum = 0.0d0
          do while (ASSOCIATED(ptrX) .and. ptrX%iRow <= nRow)
                 sum = sum + ptrX%dataVal*Y(ptrX%iRow)
              ptrX => ptrX%nextInCol
          end do
      end function ddotColSparseDense
C******************************************************* multiplyTSparseMatrix
C  PURPOSE:
C  Multiply the transpose of sparse matrix X times sparse matrix Y
C  to yield sparse matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplyTSparseMatrix
      subroutine multiplyTSparseMatrix(X, Y, Z,*)
          type(cMatrix), intent(in) :: X, Y
          type(cMatrix), intent(inout) :: Z
C  Local variables
          real(8)   values(Y%nCol), sum
          integer i, j, m, iCols(Y%nCol)
          type(cPtrCElement) :: ptrLast(Y%nCol)
            if (X%nRow /= Y%nRow) then
            call ERTN("the matrices do not conform for multiplication in
     & multiplyTSparseMatrix.",0,*99)
            end if
          call reshapeSparseMatrix(Z, X%nCol, Y%nCol,*99)
          call nullifyPtrs(Y%nCol, ptrLast)
          do i = 1, X%nCol
              if (ASSOCIATED(X%firstInCol(i)%p)) then
                  m = 0
                  do j = 1, Y%nCol
                      if (ASSOCIATED(Y%firstInCol(j)%p)) then
               sum = ddotColSparse(X%firstInCol(i)%p, Y%firstInCol(j)%p)
                          if (sum /= 0.0d0) then
                              m = m + 1
                              iCols(m) = j
                              values(m) = sum
                          end if
                      end if
                  end do
       if (m > 0) call addSparseRow(Z, m, values, i, iCols, ptrLast,*99)
              end if
          end do
      RETURN
  99  WRITE(99,'('' multiplyTSparseMatrix''\)')
      RETURN 1
      end subroutine multiplyTSparseMatrix
C******************************************************* multiplySparseTMatrix
C  PURPOSE:
C  Multiply the sparse matrix X times the transpose of the sparse
C  matrix Y to yield sparse matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseTMatrix
      subroutine multiplySparseTMatrix(X, Y, Z,*)
          type(cMatrix), intent(in) :: X, Y
          type(cMatrix), intent(inout) :: Z
C  Local variables
          real(8)   values(Y%nRow), sum
          integer i, j, m, iCols(Y%nRow)
          type(cPtrCElement) :: ptrLast(Y%nRow)
            if (X%nCol /= Y%nCol) then
          call ERTN("the matrices do not conform for multiplication in m
     &ultiplySparseTMatrix.",0,*99)
            end if
          call reshapeSparseMatrix(Z, X%nRow, Y%nRow,*99)
          call nullifyPtrs(Y%nRow, ptrLast)
          do i = 1, X%nRow
              if (ASSOCIATED(X%firstInRow(i)%p)) then
                  m = 0
                  do j = 1, Y%nRow
                      if (ASSOCIATED(Y%firstInRow(j)%p)) then
              sum = ddotRowSparse(X%firstInRow(i)%p, Y%firstInRow(j)%p)
                          if (sum /= 0.0d0) then
                              m = m + 1
                              iCols(m) = j
                              values(m) = sum
                          end if
                      end if
                  end do
       if (m > 0) call addSparseRow(Z, m, values, i, iCols, ptrLast,*99)
              end if
          end do
      RETURN
  99  WRITE(99,'('' multiplySparseTMatrix''\)')
      RETURN 1
      end subroutine multiplySparseTMatrix
C******************************************************* printSparseMatrix
C  PURPOSE:
C  A simple print out of the sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 printSparseMatrix
      subroutine printSparseMatrix(X, title)
          type(cMatrix), intent(in) :: X
          character(len=*) :: title
C  Local variables
          type(cElement), pointer :: ptrElement
          integer i, k
            character(len=200) :: buff
          write(6, *) title
          do i = 1, X%nRow
              ptrElement => X%firstInRow(i)%p
              if (ASSOCIATED(ptrElement)) then
                  do
                           buff = " "
                      write(buff,"(2i4,G12.4,2I4)") ptrElement%iRow,
     &                           ptrElement%iCol, ptrElement%dataVal,
     &                           X%firstInCol(ptrElement%iCol)%p%iCol,
     &                           X%firstInRow(ptrElement%iRow)%p%iRow
                           k = LEN_TRIM(buff)
                           if (ASSOCIATED(ptrElement%nextInCol)) then
                                k = LEN_TRIM(buff)
                                write(buff((k+2):100),"(2I4)")
     &                                ptrElement%nextInCol%iRow,
     &                                ptrElement%nextInCol%iCol
                           else
                                buff((k+2):(k+3)) = "NA"
                           end if
                           k = LEN_TRIM(buff)
                           if (ASSOCIATED(ptrElement%nextInRow)) then
                                k = LEN_TRIM(buff)
                                write(buff((k+2):200),"(2I4)")
     &                                ptrElement%nextInRow%iRow,
     &                                ptrElement%nextInRow%iCol
                           else
                                buff((k+2):(k+3)) = "NA"
                           end if
                           k = LEN_TRIM(buff)
                           write(6,*) buff(1:k)
                           ptrElement => ptrElement%nextInRow
                      if (.not. ASSOCIATED(ptrElement)) then
                          EXIT
                      end if
                  end do
              end if
          end do
      end subroutine printSparseMatrix
C******************************************************* transposeMatrix
C  PURPOSE:
C  Transpose the sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 transposeMatrix
      subroutine transposeMatrix(X)
          type(cMatrix), intent(inout) :: X
C  Local variables
          integer i, itmp
          type(cElement), pointer :: ptr, tmpPtr
          type(cPtrCElement) :: firstInRow(X%nRow)
          type(cPtrCElement) :: firstInCol(X%nCol)
C   Change the iRow, iCol, nextInRow and nextInColumn variable in each element,
C   one row at a time.  Also store the firstInRow pointers.
          do i = 1, X%nRow
              ptr => X%firstInRow(i)%p
              firstInRow(i)%p => ptr
              do while (ASSOCIATED(ptr))
                  itmp = ptr%iRow
                  ptr%iRow = ptr%iCol
                  ptr%iCol = itmp
                  tmpPtr => ptr%nextInRow
                  ptr%nextInRow => ptr%nextInCol
                  ptr%nextInCol => tmpPtr
                  ptr => tmpPtr
              end do
          end do
C   Store the firstInCol pointers
          do i = 1, X%nCol
              firstInCol(i)%p => X%firstInCol(i)%p
          end do
C   Interchange the nRow and nCol
          itmp = X%nRow
          X%nRow = X%nCol
          X%nCol = itmp
C   Set the firstInRow and firstInCol pointer
          deallocate(X%firstInRow)
          allocate(X%firstInRow(X%nRow))
          deallocate(X%firstInCol)
          allocate(X%firstInCol(X%nCol))
          do i = 1, X%nRow
              X%firstInRow(i)%p => firstInCol(i)%p
          end do
          do i = 1, X%nCol
              X%firstInCol(i)%p => firstInRow(i)%p
          end do
      end subroutine transposeMatrix
C******************************************************* nonZeroProd
C  PURPOSE:
C  See if the dot product of two columns in a sparse matrix is zero.
C  Does not check for numerical zero's, just checks for no common
C  elements in the two columns.  Here ptrOneIn and ptrTwoIn point
C  to the first element in each column.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 nonZeroProd
      function nonZeroProd(ptrOneIn, ptrTwoIn) result(out)
          type(cElement), pointer :: ptrOneIn
          type(cElement), pointer :: ptrTwoIn
C  Local variables
          integer  :: out
          type(cElement), pointer :: ptrOne, ptrTwo
          ptrOne => ptrOneIn
          ptrTwo => ptrTwoIn
          out = 0
          if (ASSOCIATED(ptrOne) .and. ASSOCIATED(ptrTwo)) then
              do
                  if (ptrOne%iRow == ptrTwo%iRow) then
                      out = 1
                      exit
                  else if (ptrOne%iRow > ptrTwo%iRow) then
                      ptrTwo => ptrTwo%nextInCol
                      if (.not. ASSOCIATED(ptrTwo)) exit
                  else
                      ptrOne => ptrOne%nextInCol
                      if (.not. ASSOCIATED(ptrOne)) exit
                  end if
              end do
          end if
      end function nonZeroProd
C******************************************************* sumRowSparse
C  PURPOSE:
C  Sums the elements in a row of a sparse matrix.
C  Here ptrIn points to the first element in the row.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 sumRowSparse
      function sumRowSparse(ptrIn) result(out)
          type(cElement), pointer :: ptrIn
          real(8)  :: out
C  Local variables
          type(cElement), pointer :: ptr
          ptr => ptrIn
          out = 0.0d0
          do
              if (ASSOCIATED(ptr)) then
                  out = out + ptr%dataVal
                  ptr => ptr%nextInRow
              else
                  exit
              end if
          end do
      end function sumRowSparse
C******************************************************* sumColSparse
C  PURPOSE:
C  Sums the elements in a column of a sparse matrix
C  Here ptrIn points to the first element in the column.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 sumColSparse
      function sumColSparse(ptrIn) result(out)
          type(cElement), pointer :: ptrIn
          real(8)  :: out
C  Local variables
          type(cElement), pointer :: ptr
          ptr => ptrIn
          out = 0.0d0
          do
              if (ASSOCIATED(ptr)) then
                  out = out + ptr%dataVal
                  ptr => ptr%nextInCol
              else
                  exit
              end if
          end do
      end function sumColSparse
C******************************************************* countRowSparse
C  PURPOSE:
C  Counts the non-null elements in a row of a sparse matrix
C  Here ptrIn points to the first element in the row.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 countRowSparse
       function countRowSparse(ptrIn) result(out)
          type(cElement), pointer :: ptrIn
          integer :: out
C  Local variables
          type(cElement), pointer :: ptr
          ptr => ptrIn
          out = 0
          do
              if (ASSOCIATED(ptr)) then
                  out = out + 1
                  ptr => ptr%nextInRow
              else
                  exit
              end if
          end do
      end function countRowSparse
C******************************************************* countColSparse
C  PURPOSE:
C  Counts the non-null elements in a column of a sparse matrix.
C  Here ptrIn points to the first element in the column.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 countColSparse
      function countColSparse(ptrIn) result(out)
          type(cElement), pointer :: ptrIn
          integer :: out
C  Local variables
          type(cElement), pointer :: ptr
          ptr => ptrIn
          out = 0
          do
              if (ASSOCIATED(ptr)) then
                  out = out + 1
                  ptr => ptr%nextInCol
              else
                  exit
              end if
          end do
      end function countColSparse
C******************************************************* getPivotOrderSparse
C  PURPOSE:
C  Gets a pivot order for computing a Cholesky decomposition of a
C  sparse matrix.  A Markowitz-like strategy is used.  The routine
C  counts the number of elements in the cross product of matrix X
C  Then finds the column order for pivoting on the cross-product by sorting
C  the number of elements associated with each column from smallest to largest.
C  iPivotOrder is the output pivot order.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getPivotOrderSparse
      subroutine getPivotOrderSparse(X, iPivotOrder,*)
          type(cMatrix), intent(in) :: X
          integer, intent(inout)  :: iPivotOrder(X%nCol)
C  Local variables
          type(cMatrix) :: countMatrix
          real(8)  :: values(X%nCol)
          integer :: iCols(X%nCol)
          integer i, j, m
          type(cPtrCElement) :: ptrLast(X%nCol)
          call setMatrix(countMatrix, X%nCol, X%nCol)
          call nullifyPtrs(X%nCol, ptrLast)
          do i = 1, X%nCol
              if (ASSOCIATED(X%firstInCol(i)%p)) then
                  m = 0
                  do j = i, X%nCol
                      if (ASSOCIATED(X%firstInCol(j)%p)) then
        if (nonZeroProd(X%firstInCol(i)%p, X%firstInCol(j)%p) > 0) then
                              m = m + 1
                              values(m) = 1.0d0
                              iCols(m) = j
                          end if
                      end if
                  end do
                if (m > 0) call addSparseRow(countMatrix, m, values, i,
     &	                        iCols, ptrLast,*99)
              end if
          end do
          do i=1, X%nCol
         values(i) = (sumRowSparse(countMatrix%firstInRow(i)%p)-1.0d0)+
     &            (sumColSparse(countMatrix%firstInCol(i)%p) - 1.0d0)
              iPivotOrder(i) = i
          end do
       IF(X%nCol.LT.1) GOTO 99
          call DSORT (values, iPivotOrder, X%nCol, 2)
      RETURN
 99   WRITE(99,'('' getPivotOrderSparse''\)')
      RETURN 1
      end subroutine getPivotOrderSparse
C******************************************************* findElement
C  PURPOSE:
C  Finds a pointer to an element in a sparse matrix.
C
C  ARGUMENTS:
C  X - Sparse matrix
C  iRow - row coordinate of desired element
C  iCol - column coordinate of desired element
C  IER  - returns with 0 if the element exists and 1 otherwise
C  ptr  - returns with the cElement pointer to X(iRow,iCol)
C         if IER=0.
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 findElement
      function findElement(X, iRow, iCol,IER) result(ptr)
          type(cMatrix), intent(in) :: X
          integer, intent(in) :: iRow
          integer, intent(in) :: iCol
          integer, intent(OUT) :: IER
          type(cElement), pointer :: ptr
C  Local variables
          type(cElement), pointer :: ptrOld
            IER = 1
          if (iRow > X%nRow .or. iCol > X%nCol .or. iRow < 1 .or.
     & iCol < 1) RETURN
          ptr => X%firstInRow(iRow)%p
          if (.not. ASSOCIATED(ptr)) then
                RETURN
          else if (ptr%iCol > iCol) then
              RETURN
          else
              ptrOld => ptr
              do
                  ptr => ptr%nextInRow
                  if (.not. ASSOCIATED(ptr).or.ptr%iCol > iCol) EXIT
                  ptrOld => ptr
              end do
              if (ptrOld%iCol /= iCol) then
                     RETURN
              else
                  ptr => ptrOld
              end if
          end if
          IER=0
            return
      end function findElement
C******************************************************* addToElementValue
C  PURPOSE:
C  Adds "value to the iRow, iCol element in sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 addToElementValue
       subroutine addToElementValue(X, iRow, iCol, value,*)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: iRow
          integer, intent(in) :: iCol
            real(8), intent(in) :: value
C  Local variables
          type(cElement), pointer :: ptrOld
            type(cElement), pointer :: ptr
          if (iRow > X%nRow .or. iCol > X%nCol .or. iRow < 1 .or.
     & iCol < 1) then
              call ERTN("The element is not in the matrix in " //
     &                      "routine addToElementValue.",0,*99)
          end if
          ptr => X%firstInRow(iRow)%p
          if (.not. ASSOCIATED(ptr)) then
                 call addNewElement(X, iRow, iCol, value,*99)
          else if (ptr%iCol > iCol) then
                 call addNewElement(X, iRow, iCol, value,*99)
          else
              ptrOld => ptr
              do
                  ptr => ptr%nextInRow
                  if (.not. ASSOCIATED(ptr).or.ptr%iCol > iCol) EXIT
                  ptrOld => ptr
              end do
              if (ptrOld%iCol /= iCol) then
                      call addNewElement(X, iRow, iCol, value,*99)
              else
                  ptrOld%dataVal = ptrOld%dataVal + value
              end if
          end if
      RETURN
 99   WRITE(99,'('' addToElementValue''\)')
      RETURN 1
      end subroutine addToElementValue
C******************************************************* addNewElement
C  PURPOSE:
C  Add an element in position (iRow, iCol) with value equal to
C  value in matrix X.
C
C  ARGUMENTS:
C  X - Sparse matrix
C  iRow - row coordinate of desired element
C  iCol - column coordinate of desired element
C  value - the value for the iRow,ICol th element of X
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 addNewElement
      subroutine addNewElement(X, iRow, iCol, value,*)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: iRow, iCol
          real(8), intent(in) :: value
C  Local variables
          integer k
          type(cElement), pointer :: ptrNew, ptr, ptrOld
          if (iRow > X%nRow .or. iCol > X%nCol .or. iRow < 1
     &                             .or. iCol < 1) then
              call ERTN("The element is not in the matrix in " //
     &                           "routine addNewElement.",0,*99)
          end if
C   Correct the row pointers
          ptr => X%firstInRow(iRow)%p
          if (.not. ASSOCIATED(ptr)) then
C   Allocate the new element, and initialize its value
              ptrNew => allocateElement(k)
              if(k.ne.0) goto 99
              call setElement(ptrNew, value, iRow, iCol)
              X%firstInRow(iRow)%p => ptrNew
              nullify(ptrNew%nextInRow)
          else if (ptr%iCol > iCol) then
C   Allocate the new element, and initialize its value
              ptrNew => allocateElement(k)
              if(k.ne.0) goto 99
              call setElement(ptrNew, value, iRow, iCol)
              ptrNew%nextInRow => ptr
              X%firstInRow(iRow)%p => ptrNew
          else
              ptrOld => ptr
              do
                  ptr => ptr%nextInRow
                  if (.not. ASSOCIATED(ptr) .or. ptr%iCol > iCol) EXIT
                  ptrOld => ptr
              end do
              if (ptrOld%iCol == iCol) then
C   The element already exists, set its value, and leave
                  ptrOld%dataVal = value
                  return
              else
C   Allocate the new element, and initialize its value
                  ptrNew => allocateElement(k)
                  if(k.ne.0) goto 99
                  call setElement(ptrNew, value, iRow, iCol)
                  if (.not. ASSOCIATED(ptr)) then
                      nullify(ptrNew%nextInRow)
                  else
                      ptrNew%nextInRow => ptrOld%nextInRow
                  end if
                  ptrOld%nextInRow => ptrNew
              end if
          end if
C   Correct the column pointers
          ptr => X%firstInCol(iCol)%p
          if (.not. ASSOCIATED(ptr)) then
              X%firstInCol(iCol)%p => ptrNew
              nullify(ptrNew%nextInCol)
          else if (ptr%iRow > iRow) then
              ptrNew%nextInCol => ptr
              X%firstInCol(iCol)%p => ptrNew
          else
              ptrOld => ptr
              do
                  ptr => ptr%nextInCol
                  if (.not. ASSOCIATED(ptr) .or. ptr%iRow > iRow) EXIT
                  ptrOld => ptr
              end do
              if (.not. ASSOCIATED(ptr)) then
                  nullify(ptrNew%nextInCol)
              else
                  ptrNew%nextInCol => ptrOld%nextInCol
              end if
              ptrOld%nextInCol => ptrNew
          end if
      RETURN
 99   WRITE(99,'('' addNewElement''\)')
      RETURN 1
      end subroutine addNewElement
C******************************************************* addOldElement
C  PURPOSE:
C  Sets the value of an element in position (iRow, iCol) of
C  sparse matrix X equal to value.
C
C  ARGUMENTS:
C  X - Sparse matrix
C  iRow - row coordinate of desired element
C  iCol - column coordinate of desired element
C  value - the value for the iRow,ICol th element of X
C
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 addOldElement
      subroutine addOldElement(X, iRow, iCol, value, ptrNew,*)
          type(cMatrix), intent(inout) :: X
          integer, intent(in) :: iRow, iCol
          real(8), intent(in) :: value
            type(cElement), intent(out), target :: ptrNew
C  Local variables
          type(cElement), pointer :: ptr, ptrOld
          if (iRow > X%nRow .or. iCol > X%nCol .or. iRow < 1
     &                             .or. iCol < 1) then
              call ERTN("The element is not in the matrix in " //
     &                           "routine addNewElement.",0,*99)
          end if
C   Correct the row pointers
          ptr => X%firstInRow(iRow)%p
          if (.not. ASSOCIATED(ptr)) then
C   Initialize ptrNew values
              call setElement(ptrNew, value, iRow, iCol)
              X%firstInRow(iRow)%p => ptrNew
              nullify(ptrNew%nextInRow)
          else if (ptr%iCol > iCol) then
C   Initialize ptrNew values
              call setElement(ptrNew, value, iRow, iCol)
              ptrNew%nextInRow => ptr
              X%firstInRow(iRow)%p => ptrNew
          else
              ptrOld => ptr
              do
                  ptr => ptr%nextInRow
                  if (.not. ASSOCIATED(ptr) .or. ptr%iCol > iCol) EXIT
                  ptrOld => ptr
              end do
              if (ptrOld%iCol == iCol) then
                call ERTN("the element is already in the matrix.",0,*99)
                  return
              else
C   Initialize ptrNew values
                  call setElement(ptrNew, value, iRow, iCol)
                  if (.not. ASSOCIATED(ptr)) then
                      nullify(ptrNew%nextInRow)
                  else
                      ptrNew%nextInRow => ptrOld%nextInRow
                  end if
                  ptrOld%nextInRow => ptrNew
              end if
          end if
C   Correct the column pointers
          ptr => X%firstInCol(iCol)%p
          if (.not. ASSOCIATED(ptr)) then
              X%firstInCol(iCol)%p => ptrNew
              nullify(ptrNew%nextInCol)
          else if (ptr%iRow > iRow) then
              ptrNew%nextInCol => ptr
              X%firstInCol(iCol)%p => ptrNew
          else
              ptrOld => ptr
              do
                  ptr => ptr%nextInCol
                  if (.not. ASSOCIATED(ptr) .or. ptr%iRow > iRow) EXIT
                  ptrOld => ptr
              end do
              if (.not. ASSOCIATED(ptr)) then
                  nullify(ptrNew%nextInCol)
              else
                  ptrNew%nextInCol => ptrOld%nextInCol
              end if
              ptrOld%nextInCol => ptrNew
          end if
      RETURN
 99   WRITE(99,'('' addOldElement''\)')
      RETURN 1
      end subroutine addOldElement
C******************************************************* setSparseMatrix
C  PURPOSE:
C  Sets all of the elements in a sparse matrix to the value "value".
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 setSparseMatrix
      subroutine setSparseMatrix(X, value)
          type(cMatrix), intent(inout) :: X
          real(8), intent(in) :: value
C  Local variables
          integer :: i
          type(cElement), pointer :: ptr
          do i = 1, X%nRow
              ptr => X%firstInRow(i)%p
              do while (ASSOCIATED(ptr))
                  ptr%dataVal = value
                  ptr => ptr%nextInRow
              end do
          end do
      end subroutine setSparseMatrix
C******************************************************* copySubsetSparseMatrix
C  PURPOSE:
C Creates a copy of the rectangular submatrix of X defined by the
C firstRow through lastRow rows and firstCol through lastCol
C columns, and stores the result in Y.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 copySubsetSparseMatrix
      subroutine copySubsetSparseMatrix(X, firstRow, lastRow, firstCol,
     &       lastCol, Y, *)
          type(cMatrix), intent(in) :: X
          integer, intent(in) :: firstRow
          integer, intent(in) :: lastRow
          integer, intent(in) :: firstCol
          integer, intent(in) :: lastCol
          type(cMatrix), intent(inout) :: Y
C  Local variables
          integer :: i, m, iCols(lastCol-firstCol+1)
            real(8) :: values(lastCol-firstCol+1)
            type(cElement), pointer :: ptr
          type(cPtrCElement) :: ptrLast(lastCol-firstCol+1)
      if (firstRow < 1.or.firstRow > lastRow.or.lastRow > X%nRow) then
              call ERTN("The rows in the cop are not valid in copySubset
     &SparseMatrix.",0,*99)
          end if
         if (firstCol < 1.or.firstCol > lastCol.or.lastCol > X%nCol)then
              call ERTN("The columns in the copy are not valid in copySu
     &bsetSparseMatrix.",0,*99)
          end if
            call reshapeSparseMatrix(Y, lastRow-firstRow+1,
     & lastCol-firstCol+1,*99)
            call nullifyPtrs(lastCol-firstCol+1,ptrLast)
C   Add the rows in X to Y, one row at a time
            do i = firstRow, lastRow
                 m = 0
                 ptr => X%firstInRow(i)%p
                 do while (ASSOCIATED(ptr) .and. ptr%iCol <= lastCol)
                      if (ptr%iCol >= firstCol) then
                           m = m + 1
                           values(m) = ptr%dataVal
                           iCols(m) = ptr%iCol-firstCol+1
                      end if
                      ptr => ptr%nextInRow
                 end do
                 if (m > 0) call addSparseRow(Y,m,values,i-firstRow+1,
     & iCols, ptrLast,*99)
            end do
      RETURN
 99   WRITE(99,'('' copySubsetSparseMatrix''\)')
      RETURN 1
      end subroutine copySubsetSparseMatrix
C******************************************************* copySubsetRowColSparse
C  PURPOSE:
C  Creates a copy of the square matrix of X defined by omitting
C  the rows and columns in firstOmit through lastOmit rows, and
C  stores the result in Y.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 01/11/04 copySubsetRowColSparse
      subroutine copySubsetRowColSparse(X, firstOmit, lastOmit, Y, *)
          type(cMatrix), intent(in) :: X
          integer, intent(in) :: firstOmit
          integer, intent(in) :: lastOmit
          type(cMatrix), intent(inout) :: Y
C  Local variables
          integer :: i, m, iCols(X%nRow-(lastOmit-firstOmit)-1)
            real(8) :: values(X%nRow-(lastOmit-firstOmit)-1)
            type(cElement), pointer :: ptr
          type(cPtrCElement) :: ptrLast(X%nRow-(lastOmit-firstOmit)-1)
          if (firstOmit < 1 .or. firstOmit > lastOmit .or.
     & lastOmit > X%nRow) then
              call ERTN("The rows in the copy are not valid.",0,*99)
          end if
          if (X%nRow /= X%nCol) then
              call ERTN("A square matrix is required.",0,*99)
          end if
            call reshapeSparseMatrix(Y, X%nRow-(lastOmit-firstOmit)-1,
     & X%nRow-(lastOmit-firstOmit)-1, *99)
            call nullifyPtrs(X%nRow-(lastOmit-firstOmit)-1,ptrLast)
C   Add the rows in X to Y, one row at a time
            do i = 1, firstOmit - 1
                 m = 0
                 ptr => X%firstInRow(i)%p
                 do while (ASSOCIATED(ptr))
               if (ptr%iCol < firstOmit .or. ptr%iCol > lastOmit) then
                           m = m + 1
                           values(m) = ptr%dataVal
                           if (ptr%iCol < firstOmit) then
                                iCols(m) = ptr%iCol
                           else
                       iCols(m) = ptr%iCol - (lastOmit - firstOmit + 1)
                           end if
                      end if
                      ptr => ptr%nextInRow
                 end do
       if (m > 0) call addSparseRow(Y, m, values, i, iCols, ptrLast,*99)
            end do
            do i = lastOmit+1, X%nRow
                 m = 0
                 ptr => X%firstInRow(i)%p
                 do while (ASSOCIATED(ptr))
                 if (ptr%iCol < firstOmit .or. ptr%iCol > lastOmit) then
                           m = m + 1
                           values(m) = ptr%dataVal
                           if (ptr%iCol < firstOmit) then
                                iCols(m) = ptr%iCol
                           else
                        iCols(m) = ptr%iCol - (lastOmit - firstOmit + 1)
                           end if
                      end if
                      ptr => ptr%nextInRow
                 end do
                 if (m > 0) call addSparseRow(Y, m, values,
     &                 i - (lastOmit-firstOmit+1), iCols, ptrLast,*99)
            end do
      return
  99  write(99,'('' copySubsetRowColSparse''\)')
      return 1
      end subroutine copySubsetRowColSparse
C******************************************************* copySubsetColSparse
C  PURPOSE:
C Creates a copy of the square matrix of X defined by omitting
C the columns firstOmit through lastOmit rows, and
C stores the result in Y.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 01/11/04 copySubsetColSparse
       subroutine copySubsetColSparse(X, firstOmit, lastOmit, Y, *)
          type(cMatrix), intent(in) :: X
          integer, intent(in) :: firstOmit
          integer, intent(in) :: lastOmit
          type(cMatrix), intent(inout) :: Y
C  Local variables
          integer :: i, m, iCols(X%nRow-(lastOmit-firstOmit)-1)
            real(8) :: values(X%nRow-(lastOmit-firstOmit)-1)
            type(cElement), pointer :: ptr
          type(cPtrCElement) :: ptrLast(X%nRow-(lastOmit-firstOmit)-1)
          if (firstOmit < 1 .or. firstOmit > lastOmit .or.
     & lastOmit > X%nRow) then
              call ERTN("The rows in the copy are not valid.",0,*99)
          end if
          if (X%nRow /= X%nCol) then
              call ERTN("A square matris is required.",0,*99)
          end if
         call reshapeSparseMatrix(Y, X%nRow,
     & X%nRow-(lastOmit-firstOmit)-1, *99)
            call nullifyPtrs(X%nRow-(lastOmit-firstOmit)-1,ptrLast)
C   Add the rows in X to Y, one row at a time
            do i = 1, X%nRow
                 m = 0
                 ptr => X%firstInRow(i)%p
                 do while (ASSOCIATED(ptr))
                if (ptr%iCol < firstOmit .or. ptr%iCol > lastOmit) then
                           m = m + 1
                           values(m) = ptr%dataVal
                           if (ptr%iCol < firstOmit) then
                                iCols(m) = ptr%iCol
                           else
                        iCols(m) = ptr%iCol - (lastOmit - firstOmit + 1)
                           end if
                      end if
                      ptr => ptr%nextInRow
                 end do
       if (m > 0) call addSparseRow(Y, m, values, i, iCols, ptrLast,*99)
            end do
      return
 99   write(99,'('' copySubsetColSparse''\)')
      return 1
      end subroutine copySubsetColSparse
C******************************************************* sparseColSaxpy
C  PURPOSE:
C  Do a sparse matrix saxpy (y = ax + y) operation on two columns
C  in two sparse matrices. Here ptrXIn points to the first element
C  in the first column, "a" is the multiplication scalar, Y is the
C  sparse matrix containing the second column, and iCol is the
C  column is Y.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 sparseColSaxpy
      subroutine sparseColSaxpy(ptrXIn, a, Y, iCol,*)
          type(cElement), pointer :: ptrXIn
          real(8) :: a
          type(cMatrix), intent(inout) :: Y
          integer, intent(in) :: iCol
C  Local variables
          type(cElement), pointer :: ptrX, ptrY
          real(8) tmp
          ptrX => ptrXIn
          ptrY => Y%firstInCol(iCol)%p
          do while (ASSOCIATED(ptrX))
              tmp = a*ptrX%dataVal
              do
                  if(.not. ASSOCIATED(ptrY)) then
                      call addNewElement(Y, ptrX%iRow, iCol, tmp,*99)
                      ptrX => ptrX%nextInCol
                           EXIT
                  else if (ptrY%iRow < ptrX%iRow) then
C   Increment the Y column pointer and continue the inner loop.
                      ptrY => ptrY%nextInCol
                  else if (ptrY%iRow == ptrX%iRow) then
                      ptrY%dataVal = ptrY%dataVal + tmp
                      ptrX => ptrX%nextInCol
                      EXIT
                  else if (ptrY%iRow > ptrX%iRow) then
                      call addNewElement(Y, ptrX%iRow, iCol, tmp,*99)
                      ptrX => ptrX%nextInCol
                           EXIT
                  end if
              end do
          end do
      RETURN
  99  WRITE(99,'('' sparseColSaxpy''\)')
      RETURN 1
      end subroutine sparseColSaxpy
C******************************************************* gramSchmidtSparse
C  PURPOSE:
C  Gram-Schmidt orthogonalization of the n columns specified by
C  iPivotOrder in the sparse matrix X, and, optionally, to the
C  sparse matrix Y and the dense matrix yy with nCol columns.
C  Argument L is the Cholesky factor for the pivoted columns in X,
C  stored such that the first row and column corresonds to
C  iPivotOrder(1), the second row and column to iPivotOrder(2), etc.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 gramSchmidtSparse
      subroutine gramSchmidtSparse(X, n, iPivotOrder,L,IER,Y,LY,nCol,
     &       yy, Lyy)
          type(cMatrix),intent(inout) :: X
          integer, intent(in) :: n
          integer, intent (in) :: iPivotOrder(:)
          type(cMatrix), intent(inout) :: L
          integer, intent(OUT) :: IER
          type(cMatrix), intent(inout), optional :: Y
          type(cMatrix), intent(inout), optional :: LY
          integer, intent(in), optional :: nCol
          real(8), intent(inout), optional :: yy(X%nRow,*)
          real(8), intent(inout), optional :: Lyy(n,*)
C  Local variables
          intrinsic MAX
          type(cElement), pointer :: ptrX, ptrY
          integer j, k, m
          real(8) sxy, sxx
          real(8)  :: values(X%nCol)
          real(8), allocatable :: valuesY(:)
          integer :: iCols(X%nCol)
          integer, allocatable :: iColsY(:)
          type(cPtrCElement):: ptrLast(X%nCol)
          type (cPtrCElement), allocatable :: ptrLastY(:)
          intrinsic SQRT
        IER=0
          call reshapeSparseMatrix(L, n, n,*99)
          call nullifyPtrs(X%nCol, ptrLast)
          if (PRESENT(Y)) then
              if (.not. PRESENT(LY)) then
                  call ERTN("gramSchmidt argument LY must be specified i
     &f argument Y is given.",0,*99)
              end if
              call reshapeSparseMatrix(LY, n, Y%nCol,*99)
              call nullifyPtrs(Y%nCol, ptrLastY)
              allocate(valuesY(Y%nCol))
              allocate(iColsY(Y%nCol))
          end if
          if (PRESENT(yy)) then
              if (.not. (PRESENT(Lyy) .and. PRESENT(nCol))) then
                call ERTN("gramSchmidt arguments nCol and Lyy must be sp
     &ecified if argument yy is given.",0,*99)
              end if
          end if
          do k = 1, n
              ptrX => X%firstInCol(iPivotOrder(k))%p
              sxx = ddotColSparse(ptrX, ptrX)
              if (sxx > 0.0d0) then
                  m = 1
                  sxx = SQRT(sxx);
                  values(m) = sxx
                  iCols(m) = k
                  do while (ASSOCIATED(ptrX))
                      ptrX%dataVal = ptrX%dataVal/sxx
                      ptrX => ptrX%nextInCol
                  end do
                  do j = k+1, n
                      ptrX => X%firstInCol(iPivotOrder(k))%p
                      ptrY => X%firstInCol(iPivotOrder(j))%p
                      sxy = ddotColSparse(ptrX, ptrY)
                      if (sxy /= 0.0d0) then
                          m = m + 1
                          values(m) = sxy
                          iCols(m) = j
                 call sparseColSaxpy(ptrX, -sxy, X, iPivotOrder(j),*99)
                      end if
                  end do
       if (m > 0) call addSparseRow(L, m, values, k, iCols, ptrLast,*99)
C   Now apply to Y
                  if (PRESENT(Y)) then
                      m = 0
                      do j = 1, Y%nCol
                          ptrX => X%firstInCol(iPivotOrder(k))%p
                          ptrY => Y%firstInCol(j)%p
                          sxy = ddotColSparse(ptrX, ptrY)
                          if (sxy /= 0.0d0) then
                              call sparseColSaxpy(ptrX, -sxy, Y, j,*99)
                              m = m + 1
                              iColsY(m) = j
                              valuesY(m) = sxy
                          end if
                      end do
                      if (m > 0) call addSparseRow(LY, m, valuesY, k,
     & iColsY, ptrLastY,*99)
                  end if
                  if (PRESENT(yy)) then
                      do j = 1, nCol
                          ptrX => X%firstInCol(iPivotOrder(k))%p
                          sxy = 0.0d0
                          do while (ASSOCIATED(ptrX))
                              sxy = sxy + ptrX%dataVal*yy(ptrX%iRow,j)
                                  ptrX => ptrX%nextInCol
                          end do
                          Lyy(k,j) = sxy
                          if (sxy /= 0.0d0) then
                              ptrX => X%firstInCol(iPivotOrder(k))%p
                              do while (ASSOCIATED(ptrX))
                   yy(ptrX%iRow,j) = yy(ptrX%iRow,j) - sxy*ptrX%dataVal
                                  ptrX => ptrX%nextInCol
                              end do
                          end if
                      end do
                  end if
              end if
          end do
      RETURN
  99  WRITE(99,'('' gramSchmidtSparse''\)')
      IER=1
      RETURN
      end subroutine gramSchmidtSparse
C******************************************************* backsolveSparseTranspose
C  PURPOSE:
C  Backsolve on the transpose of sparse triangular matrix R with
C  sparse right-hand-side Y. iPivotOrder gives the pivoting order,
C  and the solution is returned in dense matrix bFull, or sparse
C  matrix bSparse.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparseTranspose
      subroutine backsolveSparseTranspose(R, Y, iPivotOrder,IER,bFull,
     &       bSparse)
          type(cMatrix), intent(in) :: R
          type(cMatrix), intent(in) :: Y
          integer iPivotOrder(R%nRow)
          integer, intent(out) :: IER
          real(8), intent(out), optional :: bFull(R%nCol, Y%nCol)
          type(cMatrix), intent(inout), optional :: bSparse
C  Local variables
          type(cElement), pointer :: ptr
          real(8) :: tempVec(Y%nRow)
          real(8)  temp
          integer i, j, k, m
          type(cPtrCElement), allocatable :: ptrLast(:)
          real(8), allocatable :: tempVec2(:)
          integer, allocatable :: iRows(:)
            IER=0
          if (.not. PRESENT(bFull) .and. .not. PRESENT(bSparse)) then
              call ERTN("The result is not to be saved in backsolveSpars
     &eTranspose.",0,*99)
          end if
          if (PRESENT(bSparse)) then
              call reshapeSparseMatrix(bSparse, R%nCol, Y%nCol,*99)
              allocate(ptrLast(R%nCol))
              call nullifyPtrs(R%nCol, ptrLast)
              allocate(tempVec2(R%nCol))
              allocate(iRows(R%nCol))
          end if
C   Loop over the columns in the right-hand-side, Y
          do j = 1, Y%nCol
C   Initialize tempVec with the sorted Y values
              tempVec = 0.0d0
              ptr => Y%firstInCol(j)%p
              do while (ASSOCIATED(ptr))
                  tempVec(ptr%iRow) = ptr%dataVal
                  ptr => ptr%nextInCol
              end do
C   Forward elimination to solve t(R) b = Y
              do i = 1, R%nCol
C   Skip this step if the right-hand-side element is zero.
                      if (ASSOCIATED(R%firstInRow(i)%p)) then
                           if ((tempVec(i)) /= 0.0d0) then
                           ptr => R%firstInRow(i)%p
                            temp = ptr%dataVal
                             tempVec(i) = tempVec(i)/temp
                               ptr => ptr%nextInRow
                           do while (ASSOCIATED(ptr))
          tempVec(ptr%iCol) = tempVec(ptr%iCol) - ptr%dataVal*tempVec(i)
                                 ptr => ptr%nextInRow
                              end do
                           end if
                      else
                           tempVec(i) = 0.0d0
                      end if
              end do
C   Unscramble the tempVec vector and place it in dense matix b
              if (PRESENT(bFull)) then
                  do i = 1, R%nCol
                      bFull(iPivotOrder(i),j) = tempVec(i)
                  end do
              end if
              if (PRESENT(bSparse)) then
                  tempVec2 = 0.0d0
                  do i = 1, R%nCol
                      tempVec2(iPivotOrder(i)) = tempVec(i)
                  end do
                  m = 0
                  do i = 1, R%nCol
                      if (tempVec2(i) /= 0.0d0) then
                          m = m + 1
                          tempVec(m) = tempVec2(i)
                          iRows(m) = i
                      end if
                  end do
                  if (m > 0) call addSparseColumn(bSparse, m, tempVec,
     & iRows, j, ptrLast,*99)
              end if
          end do
C  Deallocate any allocated arrays
          if (PRESENT(bSparse)) then
              deallocate(tempVec2)
              deallocate(iRows)
              deallocate(ptrLast)
          end if
      return
  99  write(99,'('' backsolveSparseTranspose''\)')
      IER=1
      return
      end subroutine backsolveSparseTranspose
C******************************************************* backsolveSparseTransposeUn
C  PURPOSE:
C  Backsolve on the transpose of sparse triangular matrix R with
C  sparse right-hand-side Y. iPivotOrder gives the pivoting order, &
C  and the solution is returned in dense matrix bFull, or sparse matrix
C  bSparse.  It is assumed that Y is not in the correct pivot order on
C  input.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparseTransposeUn
      subroutine backsolveSparseTransposeUn(R,Y,iPivotOrder,IER,bFull,
     &       bSparse)
          type(cMatrix), intent(in) :: R
          type(cMatrix), intent(in) :: Y
          integer iPivotOrder(R%nRow)
          integer, intent(OUT) :: IER
          real(8), intent(out), optional :: bFull(R%nCol, Y%nCol)
          type(cMatrix), intent(inout), optional :: bSparse
C  Local variables
          type(cElement), pointer :: ptr
          real(8) :: tempVec(Y%nRow)
          real(8)  temp
          integer i, j, k, m
          type(cPtrCElement), allocatable :: ptrLast(:)
          real(8) :: tempVec2(R%nCol)
          integer, allocatable :: iRows(:)
            IER=0
          if (.not. PRESENT(bFull) .and. .not. PRESENT(bSparse)) then
              call ERTN("The result is not to be saved in backsolveSpars
     &eTranspose.",0,*99)
          end if
          if (PRESENT(bSparse)) then
              call reshapeSparseMatrix(bSparse, R%nCol, Y%nCol,*99)
              allocate(ptrLast(R%nCol))
              call nullifyPtrs(R%nCol, ptrLast)
              allocate(iRows(R%nCol))
          end if
C   Loop over the columns in the right-hand-side, Y
          do j = 1, Y%nCol
C   Initialize tempVec with the sorted Y values
              tempVec2 = 0.0d0
              ptr => Y%firstInCol(j)%p
              do while (ASSOCIATED(ptr))
                  tempVec2(ptr%iRow) = ptr%dataVal
                  ptr => ptr%nextInCol
              end do
                 do i = 1, R%nCol
                      tempVec(i) = tempVec2(iPivotOrder(i))
                 end do
C   Forward elimination to solve t(R) b = Y
              do i = 1, R%nCol
C   Skip this step if the right-hand-side element is zero.
                      if (ASSOCIATED(R%firstInRow(i)%p)) then
                           if ((tempVec(i)) /= 0.0d0) then
                           ptr => R%firstInRow(i)%p
                            temp = ptr%dataVal
                             tempVec(i) = tempVec(i)/temp
                              ptr => ptr%nextInRow
                           do while (ASSOCIATED(ptr))
          tempVec(ptr%iCol) = tempVec(ptr%iCol) - ptr%dataVal*tempVec(i)
                                 ptr => ptr%nextInRow
                              end do
                           end if
                      else
                           tempVec(i) = 0.0d0
                      end if
              end do
C   Unscramble the tempVec vector and place it in dense matix b
              if (PRESENT(bFull)) then
                  do i = 1, R%nCol
                      bFull(iPivotOrder(i),j) = tempVec(i)
                  end do
              end if
              if (PRESENT(bSparse)) then
                  tempVec2 = 0.0d0
                  do i = 1, R%nCol
                      tempVec2(iPivotOrder(i)) = tempVec(i)
                  end do
                  m = 0
                  do i = 1, R%nCol
                      if (tempVec2(i) /= 0.0d0) then
                          m = m + 1
                          tempVec(m) = tempVec2(i)
                          iRows(m) = i
                      end if
                  end do
                  if (m > 0) call addSparseColumn(bSparse, m, tempVec,
     & iRows, j, ptrLast,*99)
              end if
          end do
C   Deallocate any allocated arrays
          if (PRESENT(bSparse)) then
              deallocate(iRows)
              deallocate(ptrLast)
          end if
      return
 99   write(99,'('' backsolveSparseTransposeUn''\)')
      IER=1
      return
      end subroutine backsolveSparseTransposeUn
C******************************************************* backsolveSparseTransposeSpecial
C  PURPOSE:
C  Backsolve on the transpose of sparse triangular matrix R with
C  sparse right-hand-side Y.  Adjust for the pivot order in
C  iPivotOrder (Y has not been pivoted). Multiply the appropriate
C  elements by GChol.  The solution is returned in sparse matrix
C  bSparse.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparseTransposeSpecial
      subroutine backsolveSparseTransposeSpecial(R,iCol0,iCol1,nColZ,Y,
     &       iPivotOrder, GChol, iPivotOrderG, bSparse,*)
          type(cMatrix), intent(in) :: R
            integer, intent(in) :: iCol0
            integer, intent(in) :: iCol1
            integer, intent(in) :: nColZ
          type(cMatrix), intent(in) :: Y
          integer iPivotOrder(R%nRow)
            type(cMatrix), intent(in) :: GChol
            integer, intent(in) :: iPivotOrderG(nColZ)
          type(cMatrix), intent(inout) :: bSparse
C  Local variables
          type(cElement), pointer :: ptr
          real(8) :: tempVec(Y%nRow)
          real(8)  temp
          integer i, j, k, m
          type(cPtrCElement) :: ptrLast(R%nCol)
          real(8) :: tempVec2(R%nCol)
          integer :: iRows(R%nCol)
C   nColZ = Y%nCol - iCol1 + 1
          call reshapeSparseMatrix(bSparse, R%nRow, R%nRow-iCol0+1,*99)
          call nullifyPtrs(R%nCol, ptrLast)
C   Loop over the columns in the right-hand-side, Y
          do j = iCol0, Y%nCol
C   Initialize tempVec with the sorted Y values
              tempVec = 0.0d0
                 tempVec2 = 0.0d0
              ptr => Y%firstInCol(j)%p
              do while (ASSOCIATED(ptr))
                      tempVec2(ptr%iRow) = ptr%dataVal
                  ptr => ptr%nextInCol
              end do
                 do i = 1, nColZ
                      tempVec(i) = tempVec2(iCol1+iPivotOrderG(i)-1)
                 end do
                 call multiplySparseFull(GChol, nColZ, 1,
     &                 tempVec, tempVec2(iCol1:R%nRow),*99)
C   Unscramble
                 do i = 1, nColZ
                      tempVec(iPivotOrderG(i)) = tempVec2(iCol1+i-1)
                 end do
                 tempVec2(iCol1:R%nRow) = tempVec(1:nColZ)
C   Now order tempVec for the backsolve
                 do i = 1, Y%nRow
                      tempVec(i) = tempVec2(iPivotOrder(i))
                 end do
C   Forward elimination to solves t(R) b = Y
              do i = 1, R%nCol
C   Skip this step if the right-hand-side element is zero.
                      if (ASSOCIATED(R%firstInRow(i)%p)) then
                           if ((tempVec(i)) /= 0.0d0) then
                           ptr => R%firstInRow(i)%p
                            temp = ptr%dataVal
                             tempVec(i) = tempVec(i)/temp
                              ptr => ptr%nextInRow
                               do while (ASSOCIATED(ptr))
         tempVec(ptr%iCol) = tempVec(ptr%iCol) - ptr%dataVal*tempVec(i)
                                     ptr => ptr%nextInRow
                           end do
                           end if
                      else
                           tempVec(i) = 0.0d0
                      end if
              end do
C   Unscramble the tempVec vector and place it in dense matix b
              tempVec2 = 0.0d0
              do i = 1, R%nRow
                  tempVec2(iPivotOrder(i)) = tempVec(i)
              end do
              m = 0
              do i = 1, R%nRow
                  if (tempVec2(i) /= 0.0d0) then
                      m = m + 1
                      tempVec(m) = tempVec2(i)
                      iRows(m) = i
                  end if
              end do
              if (m > 0) call addSparseColumn(bSparse,m,tempVec,iRows,
     &                      j-iCol0+1, ptrLast,*99)
C  ptr => bSparse%firstInRow(1)%p
C  ptr => ptr%nextInRow
          end do
      return
 99   write(99,'('' backsolveSparseTransposeSpecial''\)')
      return 1
      end subroutine backsolveSparseTransposeSpecial
C******************************************************* backsolveSparseDenseTranspose
C  PURPOSE:
C  Backsolve on the transpose of sparse triangular matrix R with
C  dense right-hand-side with nCol columns Y.  iPivotOrder gives
C  the pivoting order. The solution is returned in dense matrix b.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparseDenseTranspose
      subroutine backsolveSparseDenseTranspose(R,nCol,Y,iPivotOrder,b)
          type(cMatrix), intent(in) :: R
            integer :: nCol
          real(8), intent(in) :: Y(R%nCol, nCol)
          integer iPivotOrder(R%nRow)
          real(8), intent(out) :: b(R%nCol, nCol)
C  Local variables
            type(cElement), pointer :: ptr
          real(8) :: tempVec(R%nCol)
          real(8)  temp
          integer i, j
C   Loop over the columns in the right-hand-side, Y
          do j = 1, nCol
C   Initialize tempVec with the sorted Y values
              tempVec = Y(:,j)
C   Forward elimination to solves t(R) b = Y
              do i = 1, R%nCol
C   Skip this step if the right-hand-side element is zero.
                      if (ASSOCIATED(R%firstInRow(i)%p) .and.
     &                      R%firstInRow(i)%p%dataVal /= 0.0d0) then
                           if ((tempVec(i)) /= 0.0d0) then
                           ptr => R%firstInRow(i)%p
                            tempVec(i) = tempVec(i)/ptr%dataVal
                             ptr => ptr%nextInRow
                              do while (ASSOCIATED(ptr))
         tempVec(ptr%iCol) = tempVec(ptr%iCol) - ptr%dataVal*tempVec(i)
                                    ptr => ptr%nextInRow
                                end do
                           end if
                      else
                           tempVec(i) = 0.0d0
                      end if
              end do
C   Unscramble the tempVec vector and place it in dense matix b
              do i = 1, R%nCol
                  b(iPivotOrder(i),j) = tempVec(i)
              end do
          end do
      end subroutine backsolveSparseDenseTranspose
C******************************************************* backsolveSparse
C  PURPOSE:
C  Backsolve on the sparse triangular matrix R with dense
C  right-hand-side Y.  iPivotOrder gives the pivoting order, and
C  the solution is returned in dense matrix bFull, or sparse
C  matrix bSparse.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparse
      subroutine backsolveSparse(R,Y,iPivotOrder,IER,bFull,bSparse)
          type(cMatrix), intent(in) :: R
          type(cMatrix), intent(in) :: Y
          integer iPivotOrder(R%nRow)
          integer, intent(OUT) :: IER
          real(8), intent(out), optional :: bFull(R%nCol, Y%nCol)
          type(cMatrix), intent(inout), optional :: bSparse
C  Local variables
          type(cElement), pointer :: ptr
          real(8) :: tempVec(Y%nRow)
          real(8)  temp
          integer i, j, k, m
          type(cPtrCElement), allocatable :: ptrLast(:)
          real(8), allocatable :: tempVec2(:)
          integer, allocatable :: iRows(:)
            IER=0
          if (.not. PRESENT(bFull) .and. .not. PRESENT(bSparse)) then
              call ERTN("The result is not to be saved in backsolveSpars
     &eTranspose.",0,*99)
          end if
          if (PRESENT(bSparse)) then
              call reshapeSparseMatrix(bSparse, R%nCol, Y%nCol,*99)
              allocate(ptrLast(R%nCol))
              call nullifyPtrs(R%nCol, ptrLast)
              allocate(tempVec2(R%nCol))
              allocate(iRows(R%nCol))
          end if
C   Loop over the columns in the right-hand-side, Y
          do j = 1, Y%nCol
C   Initialize tempVec with the sorted Y values
              tempVec = 0.0d0
              ptr => Y%firstInCol(j)%p
              do while (ASSOCIATED(ptr))
                  tempVec(ptr%iRow) = ptr%dataVal
                  ptr => ptr%nextInCol
              end do
C   Backward substitution to solves R b = Y
              do i = R%nCol, 1, -1
                  temp = tempVec(i);
                  ptr => R%firstInRow(i)%p
                  if(ASSOCIATED(ptr)) then
                           ptr => ptr%nextInRow
                           do while (ASSOCIATED(ptr))
                           temp = temp - ptr%dataVal * tempVec(ptr%iCol)
                            ptr => ptr%nextInRow
                         end do
                          tempVec(i) = temp/(R%firstInRow(i)%p%dataVal)
                      else
                           tempVec(i) = 0.0d0
                      end if
                 end do
C   Unscramble tempVec vector and place the values in b
              if (PRESENT(bFull)) then
                  do i = 1, R%nCol
                      bFull(iPivotOrder(i),j) = tempVec(i)
                  end do
              end if
              if (PRESENT(bSparse)) then
                  tempVec2 = 0.0d0
                  do i = 1, R%nCol
                      tempVec2(iPivotOrder(i)) = tempVec(i)
                  end do
                  m = 0
                  do i = 1, R%nCol
                      if (tempVec2(i) /= 0.0d0) then
                          m = m + 1
                          tempVec(m) = tempVec2(i)
                          iRows(m) = i
                      end if
                  end do
                  if (m > 0) call addSparseColumn(bSparse, m, tempVec,
     & iRows, j, ptrLast,*99)
              end if
          end do
C   Deallocate any allocated arrays
          if (PRESENT(bSparse)) then
              deallocate(tempVec2)
              deallocate(iRows)
              deallocate(ptrLast)
          end if
      RETURN
  99  WRITE(99,'('' backsolveSparse''\)')
      IER=1
      RETURN
      end subroutine backsolveSparse
C******************************************************* backsolveSparseUn
C  PURPOSE:
C  Backsolve on the sparse triangular matrix R with dense
C  right-hand-side Y.  iPivotOrder gives the pivoting order, and
C  the solution is returned in dense matrix bFull, or sparse
C  matrix bSparse.  Y is not assumed to be in the correct order
C  on input.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparseUn
      subroutine backsolveSparseUn(R,Y,iPivotOrder,IER,bFull,bSparse)
          type(cMatrix), intent(in) :: R
          type(cMatrix), intent(in) :: Y
          integer iPivotOrder(R%nRow)
          integer, intent(OUT) :: IER
          real(8), intent(out), optional :: bFull(R%nCol, Y%nCol)
          type(cMatrix), intent(inout), optional :: bSparse
C  Local variables
          type(cElement), pointer :: ptr
          real(8) :: tempVec(Y%nRow)
          real(8)  temp
          integer i, j, k, m
          type(cPtrCElement), allocatable :: ptrLast(:)
          real(8) :: tempVec2(R%nCol)
          integer, allocatable :: iRows(:)
        IER=0
          if (.not. PRESENT(bFull) .and. .not. PRESENT(bSparse)) then
              call ERTN("The result is not to be saved in backsolveSpars
     &eTranspose.",0,*99)
          end if
          if (PRESENT(bSparse)) then
              call reshapeSparseMatrix(bSparse, R%nCol, Y%nCol,*99)
              allocate(ptrLast(R%nCol))
              call nullifyPtrs(R%nCol, ptrLast)
              allocate(iRows(R%nCol))
          end if
C   Loop over the columns in the right-hand-side, Y
          do j = 1, Y%nCol
C   Initialize tempVec with the sorted Y values
              tempVec2 = 0.0d0
              ptr => Y%firstInCol(j)%p
              do while (ASSOCIATED(ptr))
                  tempVec2(ptr%iRow) = ptr%dataVal
                  ptr => ptr%nextInCol
              end do
                 do i = 1, R%nCol
                      tempVec(i) = tempVec2(iPivotOrder(i))
                 end do
C   Backward substitution to solves R b = Y
              do i = R%nCol, 1, -1
                  temp = tempVec(i);
                  ptr => R%firstInRow(i)%p
                  if(ASSOCIATED(ptr)) then
                           ptr => ptr%nextInRow
                           do while (ASSOCIATED(ptr))
                           temp = temp - ptr%dataVal * tempVec(ptr%iCol)
                            ptr => ptr%nextInRow
                         end do
                          tempVec(i) = temp/(R%firstInRow(i)%p%dataVal)
                      else
                           tempVec(i) = 0.0d0
                      end if
                 end do
C   Unscramble tempVec vector and place the values in b
              if (PRESENT(bFull)) then
                  do i = 1, R%nCol
                      bFull(iPivotOrder(i),j) = tempVec(i)
                  end do
              end if
              if (PRESENT(bSparse)) then
                  tempVec2 = 0.0d0
                  do i = 1, R%nCol
                      tempVec2(iPivotOrder(i)) = tempVec(i)
                  end do
                  m = 0
                  do i = 1, R%nCol
                      if (tempVec2(i) /= 0.0d0) then
                          m = m + 1
                          tempVec(m) = tempVec2(i)
                          iRows(m) = i
                      end if
                  end do
                  if (m > 0) call addSparseColumn(bSparse, m, tempVec,
     & iRows, j, ptrLast,*99)
              end if
          end do
C   Deallocate any allocated arrays
          if (PRESENT(bSparse)) then
              deallocate(iRows)
              deallocate(ptrLast)
          end if
      RETURN
  99  WRITE(99,'('' backsolveSparseUn''\)')
      IER=1
      RETURN
      end subroutine backsolveSparseUn
C******************************************************* backsolveSparseDense
C  PURPOSE:
C  Backsolve on the sparse triangular matrix R with dense
C  right-hand-side Y.  iPivotOrder gives the pivoting order.
C  The solution is returned in dense matrix b.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 backsolveSparseDense
      subroutine backsolveSparseDense(R, nCol, Y, iPivotOrder, b)
          type(cMatrix), intent(in) :: R
            integer, intent(in) :: nCol
          real(8), intent(in) :: Y(R%nCol, nCol)
          integer iPivotOrder(R%nRow)
          real(8), intent(out), optional :: b(R%nCol, nCol)
C  Local variables
          type(cElement), pointer :: ptr
          real(8) :: tempVec(R%nRow)
          real(8)  temp
          integer i, j
C   Loop over the columns in the right-hand-side, Y
          do j = 1, nCol
C   Initialize tempVec with Y
              tempVec = Y(:,j)
C   Backward substitution to solves R b = Y
              do i = R%nCol, 1, -1
                  temp = tempVec(i);
                  ptr => R%firstInRow(i)%p
                  if(ASSOCIATED(ptr) .and. ptr%dataVal /= 0.0d0) then
                           ptr => ptr%nextInRow
                           do while (ASSOCIATED(ptr))
                           temp = temp - ptr%dataVal * tempVec(ptr%iCol)
                            ptr => ptr%nextInRow
                         end do
                           if (temp /= 0.0d0) then
                           tempVec(i) = temp/(R%firstInRow(i)%p%dataVal)
                           else
                                tempVec(i) = 0.0d0
                           end if
                      else
                           tempVec(i) = 0.0d0
                      end if
              end do
C   Unscramble tempVec vector and place the values in b
              do i = 1, R%nCol
                  b(iPivotOrder(i),j) = tempVec(i)
              end do
          end do
      end subroutine backsolveSparseDense
C******************************************************* copySparseMatrixNew
C  PURPOSE:
C  Copy the sparse matrix X into the sparse matrix Y.
C  The elements in Y are allocated (taken from the stack)
C  in this routine.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 copySparseMatrixNew
      subroutine copySparseMatrixNew(X, Y,*)
          type(cMatrix), intent(in) :: X
          type(cMatrix), intent(inout) :: Y
C  Local variables
          type(cElement), pointer :: ptr
          type(cPtrCElement) :: ptrLast(X%nCol)
          integer :: i, m
          integer :: iCols(X%nCol)
          real(8)    :: rowVals(X%nCol)
          call reshapeSparseMatrix(Y, X%nRow, X%nCol,*99)
          call nullifyPtrs(X%nCol, ptrLast)
          do i = 1, X%nRow
              ptr => X%firstInRow(i)%p
              m = 0
              do while(ASSOCIATED(ptr))
                  m  = m + 1
                  iCols(m) = ptr%iCol
                  rowVals(m) = ptr%dataVal
                  ptr => ptr%nextInRow
              end do
       if(m > 0) call addSparseRow(Y, m, rowVals, i, iCols, ptrLast,*99)
          end do
      RETURN
 99   WRITE(99,'('' copySparseMatrixNew''\)')
      RETURN 1
      end subroutine copySparseMatrixNew
C******************************************************* copySparseMatrix
C  PURPOSE:
C  Copy the sparse matrix X into the sparse matrix Y.
C  The elements in Y are assumed to exist in Y.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 copySparseMatrix
      subroutine copySparseMatrix(X, Y,*)
          type(cMatrix), intent(in) :: X
          type(cMatrix), intent(inout) :: Y
C  Local variables
          type(cElement), pointer :: ptrX, ptrY
          integer :: i
          if (X%nCol /= Y%nCol .or. X%nRow /= Y%nRow) then
              call ERTN("The X and Y matrices do not conform in copySpar
     &seMatrix.",0,*99)
          end if
          do i = 1, X%nRow
              ptrX => X%firstInRow(i)%p
              ptrY => Y%firstInRow(i)%p
              do while(ASSOCIATED(ptrX) .and. ASSOCIATED(ptrY))
                  if (ptrX%iCol == ptrY%iCol) then
                      ptrY%dataVal = ptrX%dataVal
                      ptrX => ptrX%nextInRow
                      ptrY => ptrY%nextInRow
                  else
                      call ERTN("Matrices X and Y do not have the same e
     &lements in copySparseMatrix.",0,*99)
                  end if
              end do
              if (ASSOCIATED(ptrX) .or. ASSOCIATED(ptrY)) then
                  call ERTN("Matrices X and Y do not have the same eleme
     &nts in copySparseMatrix.",0,*99)
              end if
          end do
      RETURN
  99  WRITE(99,'('' copySparseMatrix''\)')
      RETURN 1
      end subroutine copySparseMatrix
C******************************************************* findLastInColumnSparseMatrix
C  PURPOSE:
C  On return, ptrLast(i) points to the last element in column i
C  of sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 findLastInColumnSparseMatrix
      subroutine findLastInColumnSparseMatrix(X, ptrLast)
          type(cMatrix), intent(in) :: X
          type(cPtrCElement), intent(inout) :: ptrLast(X%nCol)
C  Local variables
          type(cElement), pointer :: ptr, ptrOld
          integer :: i
          do i = 1, X%nCol
              ptrOld => X%firstInCol(i)%p
              ptr => ptrOld
              do while (ASSOCIATED(ptr))
                  ptrOld => ptr
                  ptr => ptr%nextInCol
              end do
              ptrLast(i)%p => ptrOld
          end do
      end subroutine findLastInColumnSparseMatrix
C******************************************************* getRowElements
C  PURPOSE:
C  Stores the nVal values in row iRow of sparse matrix X in rowVals
C  with columns specified by iCols.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getRowElements
      subroutine getRowElements(X, iRow, nVals, iCols, rowVals)
          type(cMatrix), intent(in) :: X
          integer, intent(in) :: iRow
          integer, intent(out) :: nVals
          integer, intent(out) :: iCols(:)
          real(8), intent(out)  :: rowVals(:)
C  Local variables
          type(cElement), pointer :: ptr
          ptr => X%firstInRow(iRow)%p
          nVals = 0
          do while (ASSOCIATED(ptr))
              nVals = nVals + 1
              iCols(nVals) = ptr%iCol
              rowVals(nVals) = ptr%dataVal
              ptr => ptr%nextInRow
          end do
      end subroutine getRowElements
C******************************************************* sparseMatrixFromDenseMatrix
C  PURPOSE:
C  Copies the dense matrix X(nRowX,nColX) into the sparse matrix Y
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 sparseMatrixFromDenseMatrix
      subroutine sparseMatrixFromDenseMatrix(nRowX, nColX, X, firstRow,
     &            lastRow, firstCol, lastCol, Y,*)
          integer, intent(in) :: nRowX
          integer, intent(in) :: nColX
          real(8), intent(in) :: X(nRowX,nColX)
            integer :: firstRow
            integer :: lastRow
            integer :: firstCol
            integer :: lastCol
          type(cMatrix), intent(inout) :: Y
C  Local variables
          integer :: i, j, m, iCols(lastCol-firstCol+1)
          real(8) :: values(nColX)
          type(cPtrCElement) :: ptrLast(lastCol-firstCol+1)
            call reshapeSparseMatrix(Y, lastRow-firstRow+1,
     & lastCol-firstCol+1,*99)
            call nullifyPtrs(lastCol-firstCol+1,ptrLast)
          do i = firstRow, lastRow
              m = 0
              do j = firstCol, lastCol
                  if (X(i,j) /= 0.0d0) then
                      m = m + 1
                      iCols(m) = j - firstCol + 1
                      values(m) = X(i,j)
                  end if
              end do
              if (m > 0) call addSparseRow(Y, m, values, i-firstRow+1,
     &                                             iCols, ptrLast,*99)
          end do
      RETURN
 99   WRITE(99,'('' sparseMatrixFromDenseMatrix''\)')
      RETURN 1
      end subroutine sparseMatrixFromDenseMatrix
C******************************************************* multiplyZbyG
C  PURPOSE:
C  Multiply Z by the GChol matrix, and put the result in XZNew.
C  (Z is part of XZ).
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplyZbyG
      subroutine multiplyZbyG(XZ, iCol1, nObs, GChol, XZNew,*)
            type(cMatrix), intent(in) :: XZ
            integer, intent(in) :: iCol1
            integer, intent(in) :: nObs
            type(cMatrix), intent(in) :: GChol
            type(cMatrix), intent(inout) :: XZNew
C  Local variables
            type(cElement), pointer :: ptrXZ, ptrXZOld, ptrGChol
            integer :: i, j, m, iCols(XZ%nCol)
            real(8) :: sum, values(XZ%nCol)
            type(cPtrCElement) :: ptrLast(XZ%nCol)
            call reshapeSparseMatrix(XZNew, XZ%nRow, XZ%nCol,*99)
            call nullifyPtrs(XZ%nCol, ptrLast)
            do i=1, nObs
                 m = 0
                 ptrXZ => XZ%firstInRow(i)%p
                 do while (ASSOCIATED(ptrXZ).and.ptrXZ%iCol < iCol1)
                      m = m + 1
                      values(m) = ptrXZ%dataVal
                      iCols(m) = ptrXZ%iCol
                      ptrXZ => ptrXZ%nextInRow
                 end do
                 ptrXZOld => ptrXZ
                 do j = iCol1, XZ%nCol
                      ptrXZ => ptrXZOld
                      ptrGChol => GChol%firstInCol(j-iCol1+1)%p
                      sum = 0.0d0
                do while (ASSOCIATED(ptrXZ) .and. ASSOCIATED(ptrGChol))
                      if (ptrXZ%iCol == ptrGChol%iRow+iCol1-1) then
                             sum = sum + ptrXZ%dataVal*ptrGChol%dataVal
                                ptrXZ => ptrXZ%nextInRow
                                ptrGChol => ptrGChol%nextInCol
                      else if (ptrXZ%iCol > ptrGChol%iRow+iCol1-1) then
                            ptrGChol => ptrGChol%nextInCol
                      else
                                ptrXZ => ptrXZ%nextInRow
                           end if
                      end do
                      if (sum /= 0.0d0) then
                           m = m + 1
                           iCols(m) = j
                           values(m) = sum
                      end if
                 end do
                 if (m > 0) call addSparseRow(XZNew,m,values,i,iCols,
     &                 ptrLast,*99)
            end do
      RETURN
  99  WRITE(99,'('' multiplyZbyG''\)')
      RETURN 1
      end subroutine multiplyZbyG
C******************************************************* multiplyZbyGTwo
C  PURPOSE:
C  Multiply Z by the GChol matrix, and put the result in XZNew.
C  (Z is part of XZ.) Also adds the identity matrix to the lower
C  left component of XZ.
C
C  ARGUMENTS:
C  UPDATES:
C  10/27/2004 - dealt with issue of transpose
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 27/10/04 multiplyZbyGTwo
      subroutine multiplyZbyGTwo(XZ, iCol1, nColXZ, GChol, XZNew,*)
            type(cMatrix), intent(in) :: XZ
            integer, intent(in) :: iCol1
            integer, intent(in) :: nColXZ
            type(cMatrix), intent(in) :: GChol
            type(cMatrix), intent(inout) :: XZNew
C  Local variables
            type(cElement), pointer :: ptrXZ, ptrXZOld, ptrGChol
            integer :: i, j, m, iCols(nColXZ)
            real(8) :: sum, values(nColXZ)
            type(cPtrCElement) :: ptrLast(nColXZ)
            type(cMatrix) :: tmpMat
            call reshapeSparseMatrix(XZNew, XZ%nRow, XZ%nCol,*99)
            call nullifyPtrs(XZ%nCol, ptrLast)
            call setMatrix(tmpMat, XZ%nRow, XZ%nCol)
C   Post-multiply the last columns by GChol^T and save in tmpMat2
            do i=1, nColXZ
                 m = 0
                 ptrXZ => XZ%firstInRow(i)%p
                 do while (ASSOCIATED(ptrXZ) .and. ptrXZ%iCol < iCol1)
                      if (ptrXZ%iCol >= i) then
                           m = m + 1
                           values(m) = ptrXZ%dataVal
                           iCols(m) = ptrXZ%iCol
                      end if
                      ptrXZ => ptrXZ%nextInRow
                 end do
                 ptrXZOld => ptrXZ
                 do j = iCol1, XZ%nCol
C   Post-multiply by GChol^T
C   We need the last sub-matrix in its entirety
                      ptrXZ => ptrXZOld
                      ptrGChol => GChol%firstInRow(j-iCol1+1)%p
                      sum = 0.0d0
                 do while (ASSOCIATED(ptrXZ) .and. ASSOCIATED(ptrGChol))
                           if (ptrXZ%iCol == ptrGChol%iCol+iCol1-1) then
                             sum = sum + ptrXZ%dataVal*ptrGChol%dataVal
                                ptrXZ => ptrXZ%nextInRow
                                ptrGChol => ptrGChol%nextInRow
                      else if (ptrXZ%iCol > ptrGChol%iCol+iCol1-1) then
                                ptrGChol => ptrGChol%nextInRow
                           else
                                ptrXZ => ptrXZ%nextInRow
                           end if
                      end do
C   Add sum to the new matrix
                      if (sum /= 0.0d0) then
                           m = m + 1
                           iCols(m) = j
                           values(m) = sum
                      end if
                 end do
               if (m > 0) call addSparseRow(tmpMat, m, values, i, iCols,
     &                 ptrLast, *99)
            end do
C   Pre-multiply the last rows columns by GChol and add 1
C   save in XZNew
            call nullifyPtrs(XZ%nRow, ptrLast)
            do i=1, nColXZ
                 m = 0
                 ptrXZ => tmpMat%firstInCol(i)%p
                 do while (ASSOCIATED(ptrXZ) .and. ptrXZ%iRow < iCol1)
                      m = m + 1
                      values(m) = ptrXZ%dataVal
                      iCols(m) = ptrXZ%iRow
                      ptrXZ => ptrXZ%nextInCol
                 end do
                 ptrXZOld => ptrXZ
                 do j = iCol1, i
                      sum = 0.0d0
C   Pre-multiply by GChol
                      ptrXZ => ptrXZOld
                      ptrGChol => GChol%firstInRow(j-iCol1+1)%p
C   We are in the region of Z, Pre-multiply by GChol
                do while (ASSOCIATED(ptrXZ) .and. ASSOCIATED(ptrGChol))
                        if (ptrXZ%iRow == ptrGChol%iCol+iCol1-1) then
                             sum = sum + ptrXZ%dataVal*ptrGChol%dataVal
                             ptrXZ => ptrXZ%nextInCol
                             ptrGChol => ptrGChol%nextInRow
                       else if (ptrXZ%iRow > ptrGChol%iRow+iCol1-1) then
                             ptrGChol => ptrGChol%nextInRow
                        else
                             ptrXZ => ptrXZ%nextInCol
                        end if
                      end do
C   Add 1 to the diagonal element of the modified Z^T Z
                      if (j == i) sum = sum + 1.0d0
C   Add sum to the new matrix
                      if (sum /= 0.0d0) then
                           m = m + 1
                           iCols(m) = j
                           values(m) = sum
                      end if
                 end do
                 if (m > 0) call addSparseColumn(XZNew,m,values,iCols,i,
     &                      ptrLast,*99)
            end do
            call reshapeSparseMatrix(tmpMat, 0, 0, *99)
      RETURN
  99  WRITE(99,'('' multiplyZbyGTwo''\)')
      RETURN 1
      end subroutine multiplyZbyGTwo
C******************************************************* multiplySparseFull
C  PURPOSE:
C  Multiplies the sparse matrix X times the dense matrix Y (with
C  nRow rows and nCol columns) and stores the result in sparse
C  matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseFull
      subroutine multiplySparseFull(X, nRow, nCol, Y, Z,*)
            type(cMatrix), intent(in) :: X
            integer, intent(in) :: nRow
            integer, intent(in) :: nCol
            real(8), intent(in) :: Y(nRow,nCol)
            real(8), intent(out) :: Z(X%nRow, nCol)
C  Local variables
            integer :: i, j
            type(cElement), pointer :: ptr
            if (X%nCol /= nRow) then
                 call ERTN("The matrices do not conform for "//
     &               "multiplication in multiplySparseFull.",0,*99)
            end if
            do i = 1, X%nRow
                 ptr => X%firstInRow(i)%p
                 Z(i,:) = 0.0d0
                 do while (ASSOCIATED(ptr))
                      do j = 1, nCol
                           Z(i,j) = Z(i,j) + ptr%dataVal*Y(ptr%iCol,j)
                      end do
                      ptr => ptr%nextInRow
                 end do
            end do
      RETURN
 99   WRITE(99,'('' multiplySparseFull''\)')
      RETURN 1
      end subroutine multiplySparseFull
C******************************************************* multiplySparseTransposeFull
C  PURPOSE:
C  Multiplies the transpose of the sparse matrix X times the
C  dense matrix Y (with nRow rows and nCol columns) and stores
C  the result in dense matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseTransposeFull
      subroutine multiplySparseTransposeFull(X, nRow, nCol, Y, Z,*)
            type(cMatrix), intent(in) :: X
            integer, intent(in) :: nRow
            integer, intent(in) :: nCol
            real(8), intent(in) :: Y(nRow,nCol)
            real(8), intent(out) :: Z(X%nCol, nCol)
C  Local variables
            integer :: i, j
            type(cElement), pointer :: ptr
            if (X%nRow /= nRow) then
                 call ERTN("The matrices do not conform for "//
     &               "multiplication in multiplySparseFull.",0,*99)
            end if
            do i = 1, X%nCol
                 ptr => X%firstInCol(i)%p
                 Z(i,:) = 0.0d0
                 do while (ASSOCIATED(ptr))
                      do j = 1, nCol
                           Z(i,j) = Z(i,j) + ptr%dataVal*Y(ptr%iRow,j)
                      end do
                      ptr => ptr%nextInCol
                 end do
            end do
      RETURN
  99  WRITE(99,'('' multiplySparseTransposeFull''\)')
      RETURN 1
      end subroutine multiplySparseTransposeFull
C******************************************************* multiplySparseFullSparse
C  PURPOSE:
C  Multiplies the sparse matrix X times the dense matrix Y (with
C  nRow rows and nCol columns) and stores the result in sparse
C  matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseFullSparse
      subroutine multiplySparseFullSparse(X, nRow, nCol, Y, Z,*)
            type(cMatrix), intent(in) :: X
            integer, intent(in) :: nRow
            integer, intent(in) :: nCol
            real(8), intent(in) :: Y(nRow,nCol)
            type(cMatrix), intent(inout) :: Z
C  Local variables
            integer :: i, j, m
            type(cElement), pointer :: ptr
            type(cPtrCElement) :: ptrLast(nCol)
            real(8) :: values(nCol), sum
            integer :: iCols(nCol)
            if (X%nCol /= nRow) then
                 call ERTN("The matrices do not conform for "//
     &               "multiplication in multiplySparseFull.",0,*99)
            end if
            call nullifyPtrs(nCol, ptrLast)
            call clearSparseMatrix(Z,*99)
            call setMatrix(Z, X%nRow, nCol)
            do i = 1, nRow
                 m = 0
                 ptr => X%firstInRow(i)%p
                 do j = 1, nCol
                      values(j) = 0.0d0
                 end do
                 do while (ASSOCIATED(ptr))
                      do j = 1, nCol
                      values(j) = values(j) + ptr%dataVal*Y(ptr%iCol,j)
                      end do
                      ptr => ptr%nextInRow
                 end do
                 do j = 1, nCol
                      if (values(j) /= 0.0d0) then
                           m = m + 1
                           values(m) = values(j)
                           iCols(m) = j
                      end if
                 end do
                 if (m > 0) call addSparseRow(Z, m, values, i,
     & iCols, ptrLast,*99)
            end do
      RETURN
 99   WRITE(99,'('' multiplySparseFullSparse''\)')
      RETURN 1
      end subroutine multiplySparseFullSparse
C******************************************************* multiplySparseSparse
C  PURPOSE:
C  Multiplies the sparse matrix X times the sparse matrix Y (with
C  nRow rows and nCol columns) and stores the result in sparse
C  matrix Z.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseSparse
      subroutine multiplySparseSparse(X, Y, Z,*)
            type(cMatrix), intent(in) :: X
            type(cMatrix), intent(in) :: Y
            type(cMatrix), intent(inout) :: Z
C  Local variables
            integer :: i, j, m
            type(cElement), pointer :: ptrX, ptrY
            type(cPtrCElement) :: ptrLast(Y%nCol)
            real(8) :: values(Y%nCol), sum
            integer :: iCols(Y%nCol)
            call reshapeSparseMatrix(Z, X%nRow, Y%nCol,*99)
            if (X%nCol /= Y%nRow) then
                 call ERTN("The matrices do not conform for "//
     &              "multiplication in multiplySparseSparse.",0,*99)
            end if
            call nullifyPtrs(Y%nCol, ptrLast)
            do i = 1, X%nRow
                 m = 0
                 do j = 1, Y%nCol
                      ptrY => Y%firstInCol(j)%p
                      ptrX => X%firstInRow(i)%p
                      sum = 0.0d0
                      do while (ASSOCIATED(ptrX) .and. ASSOCIATED(ptrY))
                           if (ptrX%iCol == ptrY%iRow) then
                                sum = sum + ptrX%dataVal*ptrY%dataVal
                                ptrY => ptrY%nextInCol
                                ptrX => ptrX%nextInRow
                           else if (ptrX%iCol > ptrY%iRow) then
                                ptrY => ptrY%nextInCol
                           else
                                ptrX => ptrX%nextInRow
                           end if
                      end do
                      if (sum /= 0.0d0) then
                           m = m + 1
                           values(m) = sum
                           iCols(m) = j
                      end if
                 end do
                 if (m > 0) call addSparseRow(Z, m, values, i, iCols,
     &                 ptrLast,*99)
            end do
      RETURN
  99  WRITE(99,'('' multiplySparseSparse''\)')
      RETURN 1
      end subroutine multiplySparseSparse
C******************************************************* applyGramSchmidtSparse
C  PURPOSE:
C  Apply a Gram Schmidt orthogonalization procedure to the columns
C  of sparse matrix Y using the previously orthogonalized sparse
C  matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 applyGramSchmidtSparse
      subroutine applyGramSchmidtSparse(X, Y,*)
            type(cMatrix),intent(in) :: X
            type(cMatrix),intent(inout) :: Y
C  Local variables
            integer :: j, k
            real(8) :: sxy
            type(cElement), pointer :: ptrX, ptrY
            do k = 1, X%nCol
                 do j = 1, Y%nCol
                     ptrX => X%firstInCol(k)%p
                    ptrY => Y%firstInCol(j)%p
                     sxy = ddotColSparse(ptrX, ptrY)
                      if (sxy /= 0.0d0) then
                          call sparseColSaxpy(ptrX, -sxy, Y, j,*99)
                   end if
                end do
            end do
      RETURN
  99  WRITE(99,'('' applyGramSchmidtSparse''\)')
      RETURN 1
      end subroutine applyGramSchmidtSparse
C******************************************************* applyGramSchmidtDense
C  PURPOSE:
C  Apply a Gram Schmidt orthogonalization procedure to the columns
C  of sparse matrix Y using the previously orthogonalized sparse
C  matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 applyGramSchmidtDense
      subroutine applyGramSchmidtDense(X, nRow, nCol, Y)
            type(cMatrix),intent(in) :: X
            integer, intent(in) :: nRow
            integer, intent(in) :: nCol
            real(8), intent(inout) :: Y(nRow,nCol)
C  Local variables
            integer :: k, j
            real(8) :: sxy
            type(cElement), pointer :: ptrX
            do k = 1, X%nCol
                 do j = 1, nCol
                  ptrX => X%firstInCol(k)%p
                   sxy = 0.0d0
                      do while (ASSOCIATED(ptrX))
                      sxy = sxy + ptrX%dataVal*Y(ptrX%iRow,j)
                           ptrX => ptrX%nextInCol
                    end do
                     if (sxy /= 0.0d0) then
                           ptrX => X%firstInCol(k)%p
                           do while (ASSOCIATED(ptrX))
                     Y(ptrX%iRow,j) = Y(ptrX%iRow,j) - sxy*ptrX%dataVal
                           ptrX => ptrX%nextInCol
                        end do
                      end if
                 end do
            end do
      end subroutine applyGramSchmidtDense
C******************************************************* getSparseColPtr
C  PURPOSE:
C  Retrieve the first in column pointer for the k-th element of
C  matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getSparseColPtr
      function getSparseColPtr(X, k) result(ptr)
            type(cMatrix), intent(in) :: X
            integer :: k
            type(cElement), pointer :: ptr
            ptr => X%firstInCol(k)%p
      end function getSparseColPtr
C******************************************************* getSparseRowPtr
C  PURPOSE:
C  Retrieve the first in row pointer for the k-th element of
C  matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getSparseRowPtr
      function getSparseRowPtr(X, k) result(ptr)
            type(cMatrix), intent(in) :: X
            integer :: k
            type(cElement), pointer :: ptr
            ptr => X%firstInRow(k)%p
      end function getSparseRowPtr
C******************************************************* getNextInCol
C  PURPOSE:
C  Retrieve a pointer to the next element in the column.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getNextInCol
      function getNextInCol(ptr) result(out)
            type(cElement), pointer :: ptr
            type(cElement), pointer :: out
            if (ASSOCIATED(ptr)) then
                 out => ptr%nextInCol
            else
                 nullify(out)
            end if
      end function getNextInCol
C******************************************************* getNextInRow
C  PURPOSE:
C  Retrieve a pointer to the next element in the row.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getNextInRow
      function getNextInRow(ptr) result(out)
            type(cElement), pointer :: ptr
            type(cElement), pointer :: out
            if (ASSOCIATED(ptr)) then
                 out => ptr%nextInRow
            else
                 nullify(out)
            end if
      end function getNextInRow
C******************************************************* getICol
C  PURPOSE:
C  Retrieve the iCol value for cElement object ptr.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getICol
      function getICol(ptr) result(out)
            type(cElement), pointer :: ptr
            integer :: out
            if (ASSOCIATED(ptr)) then
                 out = ptr%iCol
            else
                 out = 0
            end if
      end function getICol
C******************************************************* getIRow
C  PURPOSE:
C  Retrieve the iRow value for cElement object ptr.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 getIRow
      function getIRow(ptr) result(out)
            type(cElement), pointer :: ptr
            integer :: out
            if (ASSOCIATED(ptr)) then
                 out = ptr%iRow
            else
                 out = 0
            end if
      end function getIRow
C******************************************************* multiplySparseRowbyDenseColumn
C  PURPOSE:
C  Multiply a sparse row in a matrix times a dense column.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 multiplySparseRowbyDenseColumn
      subroutine multiplySparseRowbyDenseColumn(firstRow,lastRow,
     &       firstCol, lastCol, X, nCol, Y, Z)
            integer, intent(in) :: firstRow
            integer, intent(in) :: lastRow
            integer, intent(in) :: firstCol
            integer, intent(in) :: lastCol
            type(cMatrix), intent(in) :: X
            integer, intent(in) :: nCol
            real(8), intent(in):: Y(lastCol-firstCol+1,nCol)
            real(8), intent(out) :: Z(lastRow-firstRow+1,nCol)
C  Local variables
            integer :: i, j, k
            type(cElement), pointer :: ptr, ptrOld
            do i = firstRow, lastRow
                 ptrOld => X%firstInRow(i)%p
                 do j = 1, nCol
                      Z(i-firstRow+1,j) = 0.0d0
                      ptr => ptrOld
                    do while (ASSOCIATED(ptr) .and. ptr%iCol <= lastCol)
                           if (ptr%iCol >= firstCol) then
                                Z(i-firstRow+1,j) = Z(i-firstRow+1,j) +
     &                             ptr%dataVal*Y(ptr%iCol-firstCol+1,j)
                           end if
                           ptr => ptr%nextInRow
                      end do
                 end do
            end do
      end subroutine multiplySparseRowbyDenseColumn
C******************************************************* cbindSparseMatrix
C  PURPOSE:
C  Binds the columns of Y to the columns of X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 cbindSparseMatrix
      subroutine cbindSparseMatrix(X,Y,*)
            type(cMatrix), intent(inout) :: X
            type(cMatrix), intent(in) :: Y
C  Local variables
            integer :: i
            type(cElement), pointer :: ptr, ptrOld
            type(cPtrCElement) :: firstInCol(X%nCol)
            if (X%nRow /= Y%nRow) then
              call ERTN("the two matrices do not conform for column " //
     &                      "binding in cbindSparseMatrix.",0,*99)
            end if
            if (Y%nCol == 0) return
C   The row pointers
            do i = 1, X%nRow
                 ptr =>Y%firstInRow(i)%p
                 do while (ASSOCIATED(ptr))
                      ptr%iCol = ptr%iCol + X%nCol
                      ptr => ptr%nextInRow
                 end do
                 ptr => X%firstInRow(i)%p
                 ptrOld => ptr
                 do while (ASSOCIATED(ptr))
                      ptrOld => ptr
                      ptr => ptr%nextInRow
                 end do
                 if (ASSOCIATED(ptrOld)) then
                      ptrOld%nextInRow => Y%firstInRow(i)%p
                 else
                      X%firstInRow(i)%p => Y%firstInRow(i)%p
                 end if
            end do
C   The X firstInColumn pointers
            do i = 1,  X%nCol
                 firstInCol(i)%p => X%firstInCol(i)%p
            end do
            deallocate(X%firstInCol)
            allocate(X%firstInCol(X%nCol+Y%nCol))
            do i = 1, X%nCol
                 X%firstInCol(i)%p => firstInCol(i)%p
            end do
            do i = 1, Y%nCol
                 X%firstInCol(i+X%nCol)%p => Y%firstInCol(i)%p
            end do
            X%nCol = X%nCol + Y%nCol
      RETURN
 99   WRITE(99,'('' cbindSparseMatrix''\)')
      RETURN 1
      end subroutine cbindSparseMatrix
C******************************************************* printFullSparseMatrix
C  PURPOSE:
C  Prints a sparse matrix in a dense matrix format.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 printFullSparseMatrix
      subroutine printFullSparseMatrix(X, title)
            type(cMatrix), intent(in) :: X
            character(len=*), intent(in) :: title
C  Local variables
            real(8) :: values(X%nCol)
            type(cElement), pointer :: ptr
            integer :: i
            write(6,*)
            write(6,*) title
            do i = 1, X%nRow
                 values = 0
                 ptr => X%firstInRow(i)%p
                 do while (ASSOCIATED(ptr))
                      values(ptr%iCol) = ptr%dataVal
                      ptr => ptr%nextInRow
                 end do
                 write(6,9000) INT(values)
            end do
 9000 format(45I3)
      end subroutine printFullSparseMatrix
C******************************************************* computeLinearComboRows
C  PURPOSE:
C  Compute a linear combination of the rows in XZ and put the
C  result in the dense matrix linearCombo.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 computeLinearComboRows
      subroutine computeLinearComboRows(X, multiplier, linearCombo)
            type(cMatrix), intent(in) :: X
            real(8), intent(in) :: multiplier(X%nRow)
            real(8), intent(out) :: linearCombo(X%nCol)
C   local variables
            integer :: i
            type(cElement), pointer :: ptr
            linearCombo = 0.0d0
            do i = 1, X%nRow
                 ptr => X%firstInRow(i)%p
                 do while (ASSOCIATED(ptr))
                      linearCombo(ptr%iCol) = linearCombo(ptr%iCol) +
     &                      multiplier(i)*ptr%dataVal
                      ptr => ptr%nextInRow
                 end do
            end do
      end subroutine computeLinearComboRows
C******************************************************* sparseCholesky
C  PURPOSE:
C  Computes a pivoted Cholesky decomposition of a symmetric matrix
C  X yielding using the pivot order returned in iPivotOrder.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 sparseCholesky
      subroutine sparseCholesky(X, iPivotOrder, pivot,*)
          type(cMatrix), intent(inout) :: X
          integer, intent(out) :: iPivotOrder(X%nCol)
            integer, intent(in) :: pivot
C  Local variables
          real(8) DSQRT
          intrinsic DSQRT
          integer done, i, oPvt, j, k, l, m(2), maxIndex,
     &iRow(X%nRow,2), iCol(X%nRow,2)
          type(cPtrCElement) :: diag(X%nCol), tmpCPtr,
     & saveStructs(X%nRow,2)
          type(cElement), pointer :: ptr, ptrOld, ptr1, ptr2, ptrOld2
            type(cElement), pointer :: ptrFirst
          type(cMatrix) :: work
          real(8) tmp, temp, maxDiag, sumsq, tol, values(X%nRow,2)
            tol = 0.0001d0
          if (X%nRow /= X%nCol) then
              call ERTN("Cholesky factorization requires a square symmet
     &ric matrix.",0,*99)
          end if
          call setMatrix(work, X%nCol, 1)
C   Set up the temp matrix, find the diagonal elements and
C   remove the lower diagonal elements.
          do i = 1, X%nCol
                 iPivotOrder(i) = i
C   populate work
              ptr => allocateElement(k)
              if(k.ne.0) goto 99
              call setElement(ptr, 0.0d0, i, 1)
              work%firstInRow(i)%p => ptr
C find diagonal elements and remove lower portion of matrix
              ptr => X%firstInRow(i)%p
              nullify(diag(i)%p)
              do while (ASSOCIATED(ptr))
                      if (ptr%iCol >= i) then
                           if (ptr%iCol == i) then
                                diag(i)%p => ptr
                                nullify(ptr%nextInCol)
                                X%firstInRow(i)%p => ptr
                                EXIT
                           else
C Diagonal element is zero - nullify row
                                nullify(X%firstInRow(i)%p)
C First take care of the nextInCol pointers
                                ptrOld => X%firstInCol(ptr%iCol)%p
                                if (ptrOld%iRow == ptr%iRow) then
C ptr is the first element in the column
                              X%firstInCol(ptr%iCol)%p => ptr%nextInCol
                                else
C Find the column element pointing to ptr
                                     do while(ASSOCIATED(ptrOld))
                                          ptr1 => ptrOld%nextInCol
                                       if (ptr1%iRow == ptr%iRow) then
                                      ptrOld%nextInCol => ptr%nextInCol
                                               EXIT
                                          end if
                                          ptrOld => ptr1
                                     end do
                                end if
C  Now delete the element
                                ptrOld => ptr
                                ptr => ptr%nextInRow
                                call deallocateElement(ptrOld)
                           end if
                  else
                           ptrOld => ptr
                           ptr => ptr%nextInRow
                           call deallocateElement(ptrOld)
                  end if
              end do
          end do
C  The Cholesky computations
          do k = 1, X%nCol
                 ptr => diag(k)%p
                 i = k
                 do while (.not. ASSOCIATED(ptr) .and. i < X%nCol)
                      i = i + 1
                      ptr => diag(i)%p
                 end do
                 if (.not. ASSOCIATED(ptr)) then
                      EXIT
                 end if
 10               done = 1
                 maxIndex = 0
C First determine the Markowitz pivot
                 maxDiag = X%nCol*X%nCol + 1
                 maxIndex = 0
                 if (pivot /= 0) then
                      do i = k, X%nCol
                           if (ASSOCIATED(X%firstInRow(i)%p)) then
                                j = countRowSparse(X%firstInRow(i)%p) *
     &                              countColSparse(X%firstInCol(i)%p)
                                if (j < maxDiag) then
                                    if (diag(i)%p%dataVal /= 0.0d0) then
                                          maxIndex = i
                                          maxDiag = j
                                     end if
                                end if
                           end if
                      end do
                 else
                      maxIndex = k
                 end if
                 if(maxIndex == 0) cycle
                 maxDiag = diag(maxIndex)%p%dataVal
C   Check the tolerance
                 sumsq = 0.0d0
                 ptr => X%firstInCol(maxIndex)%p
                 do while (ASSOCIATED(ptr))
                      sumsq = sumsq + ptr%dataVal*ptr%dataVal
                      ptr => ptr %nextInCol
                 end do
                 if (DABS(maxDiag) < tol*DSQRT(sumsq)) then
C   The pivot element is linearly related to previous
C   columns in the matrix - delete the row and column
                      nullify(diag(maxIndex)%p)
C   First the row
                      ptr => X%firstInRow(maxIndex)%p
                      do while (ASSOCIATED(ptr))
                           ptrOld => ptr%nextInRow
C   This corrects the column pointers and
C   deallocates the element for this row
                           call deleteColElement(X, ptr)
                           ptr => ptrOld
                      end do
                      nullify(X%firstInRow(maxIndex)%p)
C   Now the column
                      ptr => X%firstInCol(maxIndex)%p
                      do while (ASSOCIATED(ptr))
                           ptrOld => ptr%nextInCol
                           call deleteRowElement(X, ptr)
                           ptr => ptrOld
                      end do
                      nullify(X%firstInCol(maxIndex)%p)
                      if (pivot == 0) cycle
                      goto 10
                 else if (maxDiag < 0.0d0) then
C   quit if the pivot element is not positive.
                 call ERTN("The matrix is not positive semi-definite in
     & sparseCholesky.",0,*99)
                 end if
                 i = iPivotOrder(k)
                 iPivotOrder(k) = iPivotOrder(maxIndex)
                 iPivotOrder(maxIndex) = i
              if (k /= maxIndex) then
C   Save column/row k and delete it from the matrix
                  ptr1 => X%firstInCol(k)%p
                   m(1) = 0
                   do while (ASSOCIATED(ptr1))
                        m(1) = m(1) + 1
                        saveStructs(m(1),1)%p => ptr1
                        iRow(m(1),1) = ptr1%iRow
                        iCol(m(1),1) = ptr1%iCol
                        values(m(1),1) = ptr1%dataVal
                        if (ptr1%iRow < k) then
C  Fix the row pointers
                             ptrOld => X%firstInRow(ptr1%iRow)%p
                             if (ptrOld%iCol == k) then
                            X%firstInRow(ptr1%iRow)%p => ptr1%nextInRow
                             else
                                  ptr => ptrOld%nextInRow
                                  do while (ASSOCIATED(ptr))
                                       if (ptr%iCol == k) EXIT
                                       ptrOld => ptr
                                       ptr => ptr%nextInRow
                                  end do
            if (ptrOld%iCol > ptr1%iCol .or. .not. ASSOCIATED(ptr)) then
                                       ptr => ptr1%nextInRow
                                    ptrOld => X%firstInRow(ptr1%iRow)%p
                                       do while (ASSOCIATED(ptrOld))
                                            ptrOld => ptrOld%nextInRow
                                          end do
            call ERTN("This pointer error should not occur", 0, *99)
                                     end if
                                     ptrOld%nextInRow => ptr%nextInRow
                                end if
                                ptr1 => ptr1%nextInCol
                           else if (ptr1%iCol > k) then
C   Fix the column pointers
                                ptrOld => X%firstInCol(ptr1%iCol)%p
                                if (ptrOld%iRow == k) then
                             X%firstInCol(ptr1%iCol)%p => ptr1%nextInCol
                                else
                                     ptr => ptrOld%nextInCol
                                     do while (ASSOCIATED(ptr))
                                          if (ptr%iRow == k) EXIT
                                          ptrOld => ptr
                                          ptr => ptr%nextInCol
                                     end do
                                     ptrOld%nextInCol => ptr1%nextInCol
                                end if
                                ptr1 => ptr1%nextInRow
                           else
                                ptr1 => ptr1%nextInRow
                           end if
                      end do
C   Save column/row maxIndex and delete it from the matrix
                  ptr2 => X%firstInCol(maxIndex)%p
                      m(2) = 0
                      do while (ASSOCIATED(ptr2))
                           m(2) = m(2) + 1
                           saveStructs(m(2),2)%p => ptr2
                           iRow(m(2),2) = ptr2%iRow
                           iCol(m(2),2) = ptr2%iCol
                           values(m(2),2) = ptr2%dataVal
                           if (ptr2%iRow < maxIndex) then
C   Fix the row pointers
                                ptrOld => X%firstInRow(ptr2%iRow)%p
                                if (ptrOld%iCol == maxIndex) then
                            X%firstInRow(ptr2%iRow)%p => ptr2%nextInRow
                                else
                                     ptr => ptrOld%nextInRow
                                     do while (ASSOCIATED(ptr))
                                          if (ptr%iCol == maxIndex) EXIT
                                          ptrOld => ptr
                                          ptr => ptr%nextInRow
                                     end do
           if (ptrOld%iCol > ptr2%iCol .or. .not. ASSOCIATED(ptr)) then
              call ERTN("This pointer error should not occur", 0, *99)
                                     end if
                                     ptrOld%nextInRow => ptr%nextInRow
                                end if
                                ptr2 => ptr2%nextInCol
                           else if (ptr2%iCol > maxIndex) then
C   Fix the column pointers
                                ptrOld => X%firstInCol(ptr2%iCol)%p
                                if (ptrOld%iRow == maxIndex) then
                          X%firstInCol(ptr2%iCol)%p => ptr2%nextInCol
                                else
                                     ptr => ptrOld%nextInCol
                                     do while (ASSOCIATED(ptr))
                                          if (ptr%iRow == maxIndex) EXIT
                                          ptrOld => ptr
                                          ptr => ptr%nextInCol
                                     end do
                                     ptrOld%nextInCol => ptr%nextInCol
                                end if
                                ptr2 => ptr2%nextInRow
                           else
                                ptr2 => ptr2%nextInRow
                           end if
                      end do
                      nullify(X%firstInRow(k)%p)
                      nullify(X%firstInRow(maxIndex)%p)
                      nullify(X%firstInCol(k)%p)
                      nullify(X%firstInCol(maxIndex)%p)
C   Now add the elements back into the matrix
                      do i = 1, m(1)
                           if (iRow(i,1) < k) then
       call addOldElement(X, iRow(i,1), maxIndex, values(i,1),
     & saveStructs(i,1)%p,*99)
                           else if (iCol(i,1) == k) then
       call addOldElement(X, maxIndex, maxIndex, values(i,1),
     & saveStructs(i,1)%p,*99)
                           else if (iCol(i,1) < maxIndex) then
       call addOldElement(X, iCol(i,1), maxIndex, values(i,1),
     & saveStructs(i,1)%p,*99)
                           else if (iCol(i,1) == maxIndex) then
       call addOldElement(X, iRow(i,1), maxIndex, values(i,1),
     & saveStructs(i,1)%p,*99)
                           else
       call addOldElement(X, maxIndex, iCol(i,1), values(i,1),
     & saveStructs(i,1)%p,*99)
                           end if
                      end do
                      do i = 1, m(2)
                           if (iRow(i,2) < k) then
       call addOldElement(X, iRow(i,2), k, values(i,2),
     & saveStructs(i,2)%p,*99)
                           else if (iRow(i,2) < maxIndex) then
       call addOldElement(X, k, iRow(i,2), values(i,2),
     & saveStructs(i,2)%p,*99)
                           else if (iCol(i,2) == maxIndex) then
       call addOldElement(X, k, k, values(i,2), saveStructs(i,2)%p,*99)
                           else
       call addOldElement(X, k, iCol(i,2), values(i,2),
     & saveStructs(i,2)%p,*99)
                           end if
                      end do
C   Now fix the "diag" pointers
                      diag(k)%p => X%firstInRow(k)%p
                      diag(maxIndex)%p => X%firstInRow(maxIndex)%p
                 end if
C   reduction step. pivoting is contained across the rows.
              tmp = DSQRT(diag(k)%p%dataVal)
              diag(k)%p%dataVal = tmp
              work%firstInRow(k)%p%dataVal = tmp
              if (k < X%nCol) then
                  ptr => diag(k)%p%nextInRow
                  if (ASSOCIATED(ptr)) then
C   Divide the row of X by tmp, and store the result in work.
                    work%firstInCol(1)%p => work%firstInRow(ptr%iCol)%p
                    ptrOld => work%firstInCol(1)%p
C   set up the work vector - divide the k-th row by tmp and
C   assign it to the work vector
                      do
                          ptr%dataVal = ptr%dataVal/tmp
                      work%firstInRow(ptr%iCol)%p%dataVal = ptr%dataVal
                          nullify(ptrOld%nextInCol)
                          ptr => ptr%nextInRow
                          if(.not.ASSOCIATED(ptr)) then
                              EXIT
                          else
                         ptrOld%nextInCol => work%firstInRow(ptr%iCol)%p
                                     ptrOld => ptrOld%nextInCol
                          end if
                      end do
C   Now set up for the SAXPY operation
                      ptr => diag(k)%p%nextInRow
C   loop over the existing row values for row k
                      do while (ASSOCIATED(ptr))
                          j = ptr%iCol
                          temp = -ptr%dataVal
C   Find first element in column j past k (the saxpy operation
C   starts with this element).
                          ptr1 => X%firstInCol(j)%p
C   Save the firstInCol pointer in ptrOld to restore it after
C   the saxpy
                                ptrFirst => ptr1
C  ptrOld points to the last element prior to the current row in column k
C  ptr1 points to the first element past (or equal to) this row
                          ptrOld => ptr1
                          do while (ASSOCIATED(ptr1))
                              if (ptr1%iRow > k) then
                                  EXIT
                              else
                                          ptrOld => ptr1
                                  ptr1 => ptr1%nextInCol
                              end if
                          end do
                          X%firstInCol(j)%p => ptr1
C   Now nullify the nextInCol pointer for the appropriate element in work
C   (other wise the saxpy operation adds unnecessary elements)
                          ptr2 => work%firstInCol(1)%p
                          ptrOld2 => ptr2
                          do while (ASSOCIATED(ptr2))
                              if (ptr2%iRow > j) then
                                  EXIT
                              else
                                  ptrOld2 => ptr2
                                  ptr2 => ptr2%nextInCol
                              endif
                          end do
                          if (ASSOCIATED(ptr2)) then
                              nullify(ptrOld2%nextInCol)
                          end if
               call sparseColSaxpy(work%firstInCol(1)%p, temp, X, j,*99)
                                if (ASSOCIATED(ptrOld)) then
                                     if (ptrOld%iRow <= k) then
                                ptrOld%nextInCol => X%firstInCol(j)%p
                                     end if
                                end if
                      if (ptrFirst%iRow < X%firstInCol(j)%p%iRow) then
                                     X%firstInCol(j)%p => ptrFirst
                                end if
                          ptrOld2%nextInCol => ptr2
                          ptr => ptr%nextInRow
                      end do
                  end if
              end if
          end do
      RETURN
  99  WRITE(99,'('' sparseCholesky''\)')
      RETURN 1
      end subroutine sparseCholesky
C******************************************************* deleteColElement
C  PURPOSE:
C  Corrects the column pointer and deallocates the element
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 deleteColElement
      subroutine deleteColElement(X, ptr)
            type(cMatrix), intent(inout) :: X
            type(cElement), pointer :: ptr
C  Local variables
            type(cElement), pointer :: ptrOld, ptrNew
            ptrNew => X%firstInCol(ptr%iCol)%p
            nullify(ptrOld)
            do while (ptrNew%iRow /= ptr%iRow)
                 ptrOld => ptrNew
                 ptrNew => ptrNew%nextInCol
            end do
            if (.not. ASSOCIATED(ptrOld)) then
                 X%firstInCol(ptr%iCol)%p => ptrNew%nextInCol
            else
                 ptrOld%nextInCol => ptrNew%nextInCol
            end if
            call deallocateElement(ptr)
      end subroutine deleteColElement
C******************************************************* deleteRowElement
C  PURPOSE:
C  Corrects the column pointer and deallocates the element
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 deleteRowElement
      subroutine deleteRowElement(X, ptr)
            type(cMatrix), intent(inout) :: X
            type(cElement), pointer :: ptr
C  Local variables
            type(cElement), pointer :: ptrOld, ptrNew
            ptrNew => X%firstInRow(ptr%iRow)%p
            nullify(ptrOld)
            do while (ptrNew%iCol /= ptr%iCol)
                 ptrOld => ptrNew
                 ptrNew => ptrNew%nextInRow
            end do
            if (.not. ASSOCIATED(ptrOld)) then
                 X%firstInRow(ptr%iRow)%p => ptrNew%nextInRow
            else
                 ptrOld%nextInRow => ptrNew%nextInRow
            end if
            call deallocateElement(ptr)
      end subroutine deleteRowElement
C******************************************************* buildNeighborMatrix
C  PURPOSE:
C  Builds a spatial neighbor matrix for rectangular lattice data.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 23/07/04 buildNeighborMatrix
      subroutine buildNeighborMatrix(N, nObs, nVar, nObsData, obsDep,
     &       dataVal, nIndVar, indVar, nParameters, parameters, *)
            type(cMatrix), intent(inout) :: N
            integer, intent(in) :: nObs
            integer, intent(in) :: nVar
            integer, intent(in) :: nObsData
            integer, intent(in) :: obsDep(nObs)
            real(8), intent(in), target :: dataVal(nObs, nVar)
            integer, intent(in) :: nIndVar
            integer, intent(in) :: indVar(nIndVar)
            integer, intent(in) :: nParameters
            real(8), intent(in) :: parameters(nParameters)
C  Local variables
            integer i, j, k, test, iRow, iCol
            real(8), pointer :: row(:)
            real(8), pointer :: column(:)
            integer ABS
            intrinsic ABS
            call setMatrix(N, nObs, nObs)
            row => dataVal(:,indVar(1))
            column => dataVal(:, indVar(2))
            do i = 1, nObs
                 do j = i+1, nObs
           iRow = ABS((row(obsDep(i))-row(obsDep(j)))*1.00001D0)
           iCol = ABS((column(obsDep(i)) - column(obsDep(j)))*1.00001d0)
                      if (iRow > 1 .or. iCol > 1) cycle
                      if (iRow == 1 .and. iCol == 1)  cycle
                      test = 0
                      if (nIndVar >= 4) then
                           do k = 4, nIndVar
                                if (dataVal(i, indVar(k)) /=
     &                              dataVal(j, indVar(k))) then
                                     test = 1
                                     EXIT
                                end if
                           end do
                      end if
                      if (test == 0) then
                           if (iRow == 1) then
                          call addNewElement(N, i, j, parameters(1),*99)
                          call addNewElement(N, j, i, parameters(1),*99)
                           else
                          call addNewElement(N, i, j, parameters(2),*99)
                          call addNewElement(N, j, i, parameters(2),*99)
                        end if
                      end if
                 end do
            end do
      RETURN
 99   WRITE(99,'('' buildNeighborMatrix''\)')
      RETURN 1
      end subroutine buildNeighborMatrix
C******************************************************* getCount
C  PURPOSE:
C  Gets the count of elements in sparse matrix X.
C
C  ARGUMENTS:
C  Author: Douglas B. Clarkson, ScienceOps
C---------------------------------------------- 01/06/06 getCount
      function getCount(X) result(count)
      type(cMatrix), intent(inout) :: X
      integer count
C  Local variables
      type(cElement), pointer :: ptr
      integer i
      count = 0
      do i=1, X%nRow
        ptr => X%firstInRow(i)%p
        do while (ASSOCIATED(ptr))
          count = count + 1
          ptr => ptr%nextInRow
        end do
      end do
      RETURN
      end function getCount
C******************************************************* READDENSEMATRIX
C  PURPOSE:
C  Read in a dense matrix and save it in a sparse array
C
C  ARGUMENTS:
C  unit - The unit number for reading commands. 
C  buff - A character buffer work space. 
C  nCol - The number of columns (and rows) in the space matrix. 
C  fileName - The name of the file containing the data. 
C  mat  - The output sparse symmetric matrix. 
C---------------------------------------------- 11/05/06 READDENSEMATRIX
      SUBROUTINE readDenseMatrix(unit, buff, nCol, fileName, mat, *)
C      use modSparseMatrix
      integer, intent(in) :: unit
      character(len=5000), intent(out) :: buff
      integer, intent(in) :: nCol
      character(len=*),intent(in) :: fileName
      type(cMatrix), intent(inout) :: mat
C Local Variables
      integer i, ii, info, iunit, ier, iCols(nCol), nChar, nVar
      real(8) values(nCol)
      type(cPtrCElement) :: ptrLast(nCol)
      iunit = 12
C Check number of columns
      if (nCol .le. 0) then
      call ERTN( "Number of columns in matrix must be non-zero.",0,*99)
      end if
C Find the unit number and if necessary open the file
      if (INDEX(fileName, "CMD") .ne. 1) then
        OPEN(iunit,FILE=fileName,STATUS='OLD',IOSTAT=ier)
        IF(ier.NE.0) CALL ERTN('OPEN ERROR ON FILE '//fileName,ier,*99)
      else
        iunit = unit
      end if
      call reshapeSparseMatrix(mat, nCol, nCol, *99)
      call nullifyPtrs(nCol,ptrLast)
C setr iCols
      do i=1, nCol
        iCols(i) = i
      end do
C Read in the data
      do i = 1, nCol
        nChar = 1
        buff = " "
        do 
          read(UNIT=iunit,FMT=9000,ADVANCE='NO',SIZE=ii,EOR=10,
     &             IOSTAT=info, err=20) buff(nChar:nChar)
9000      format(a1)
          if (info > 0) then
            call ERTN("An error has occurred in reading the "//
     &       "input line of numbers: " // TRIM(buff(1:nChar)),0,*99)
          end if
          if (buff(nChar:nChar) == '&') then
            read(unit,*)
            cycle
          end if
          nChar = nChar + 1
        end do
10      nChar = nChar - 1
C Interpret the line
        nVar = 0
        do
          buff = ADJUSTL(buff)
          ii = INDEX(buff, ' ')
          if (ii == 1) then
            ii = LEN_TRIM(buff)
            if (ii == 0) then
                EXIT
            end if
          end if
          nVar = nVar + 1
          if (nVar > nCol) call ERTN("Reading in too many values" 
     &             // " in a line of numbers.",0,*99)
          read(buff(1:ii), *, err=20) values(nVar)
          buff(1:ii) = " "
        end do
        call addSparseRow(mat, nCol, values, i, iCols, ptrLast,*99)
      end do
      if (INDEX(fileName, "CMD") .ne. 1) then
        close(UNIT=iunit)
      end if
      return
  20  call ERTN("An error has occurred in reading the "//
     &           "input line of numbers: " // TRIM(buff(1:nChar)),0,*99)
99    WRITE(99,'('' READDENSEMATRIX''\)')
      RETURN 1
      end subroutine readDenseMatrix
C******************************************************* READSPARSEMATRIX
C  PURPOSE:
C  Read in a sparse matrix and save it in a sparse array
C
C  ARGUMENTS:
C  unit - The unit number for reading commands. 
C  buff - A character buffer work space. 
C  nCol - The number of columns (and rows) in the space matrix. 
C  fileName - The name of the file containing the data.
C             or "CMD" if the file is already open on Lu unit. 
C  mat  - The output sparse symmetric matrix. 
C---------------------------------------------- 11/05/06 READSPARSEMATRIX
      SUBROUTINE readSparseMatrix(unit, buff, nCol, fileName, mat, *)
C      use modSparseMatrix
      integer, intent(in) :: unit
      character(len=5000), intent(out) :: buff
      integer, intent(in) :: nCol
      character(len=*),intent(in) :: fileName
      type(cMatrix), intent(inout) :: mat
C Local Variables
      integer ii, info, iunit, ier, iCols(nCol), nChar, nVar
      integer iRow, iRowOld
      real(8) values(nCol)
      type(cPtrCElement) :: ptrLast(nCol)
      logical eofCond;
      iunit = 12
C Check number of columns
      if (nCol .le. 0) then
      call ERTN( "Number of columns in matrix must be non-zero.",0,*99)
      end if
C Find the unit number and if necessary open the file
      if (INDEX(fileName, "CMD") .ne. 1) then
        OPEN(iunit,FILE=fileName,STATUS='OLD',IOSTAT=ier)
        IF(ier.NE.0) CALL ERTN('OPEN ERROR ON FILE '//fileName,ier,*99)
      else
        iunit = unit
      end if
      call reshapeSparseMatrix(mat, nCol, nCol, *99)
      call nullifyPtrs(nCol,ptrLast)
C Read in the data
      iRowOld = 0
      nVar = 0
      eofCond = .FALSE.
      do
        nChar = 1
        buff = " "
        if (.not. eofCond) then  
          do 
            read(UNIT=iunit,FMT=9000,ADVANCE='NO',SIZE=ii,EOR=10,
     &            IOSTAT=info, err=20, end=30) buff(nChar:nChar)
9000        format(a1)
            if (info > 0) then
              call ERTN("An error had occurred in reading the "//
     &         "input line of numbers: " // TRIM(buff(1:nChar)),0,*99)
            end if 
            if (buff(nChar:nChar) == '&') then
              read(unit,*)
              cycle
            end if
            nChar = nChar + 1
          end do
        else 
          if (nVar .gt. 0) then
            call addSparseRow(mat,nVar,values,iRowOld,iCols,ptrLast,*99) 
          end if 
          if (INDEX(fileName, "CMD") .ne. 1) then
            close(UNIT=iunit)
          end if
          return
        end if
C  End of file
30      if (info > 0) then
            call ERTN("An error had occurred in reading the "//
     &       "input line of numbers: " // TRIM(buff(1:nChar)),0,*99)
        else 
          eofCond = .TRUE.
          if (nVar .gt. 0) then
            call addSparseRow(mat,nVar,values,iRowOld,iCols,ptrLast,*99) 
          end if 
          if (INDEX(fileName, "CMD") .ne. 1) then
            close(UNIT=iunit)
          end if
        end if
        if (nChar .eq. 1) then
          return
        end if
C Process the record
10      nChar = nChar - 1
C Interpret the line
C First check for the end of the matrix
        if (iunit .eq. unit .and. INDEX(buff,"endData") .eq. 1) then
          if (nVar .gt. 0) then
            call addSparseRow(mat,nVar,values,iRowOld,iCols,ptrLast,*99)  
          end if
          return
        end if
C Now get the row and columns
        buff = ADJUSTL(buff)
        ii = INDEX(buff, ' ')
        if (ii == 1) then
          ii = LEN_TRIM(buff)
C If ii is zero we are at the end of the file but there is junk on the last line
          if (ii == 0) then
            if (nVar .gt. 0) then
            call addSparseRow(mat,nVar,values,iRowOld,iCols,ptrLast,*99)  
            end if
            return
          end if
        end if
        read(buff(1:ii), *, err=20) iRow
        if (iRow .ne. iRowOld) then
          if (nVar .gt. 0) then
            call addSparseRow(mat,nVar,values,iRowOld,iCols,ptrLast,*99)  
          end if
          iRowOld = iRow
          nVar = 0
        end if
        buff(1:ii) = " "
        buff = ADJUSTL(buff)
        ii = INDEX(buff, ' ')
        if (ii == 1) then
          ii = LEN_TRIM(buff)
          if (ii == 0) then
              goto 20
          end if
        end if
        nVar = nVar + 1
        read(buff(1:ii), *, err=20) iCols(nVar)
        buff(1:ii) = " "
        buff = ADJUSTL(buff)
        ii = INDEX(buff, ' ')
        if (ii == 1) then
          ii = LEN_TRIM(buff)
          if (ii == 0) then
              goto 20
          end if
        end if
        read(buff(1:ii), *, err=20) values(nVar)
      end do
      if (INDEX(fileName, "CMD") .ne. 1) then
        close(UNIT=iunit)
      end if
      return
  20  call ERTN("An error had occurred in reading the "//
     &        "input line of numbers: " // TRIM(buff(1:nChar)),0,*99)
99    WRITE(99,'('' READSPARSEMATRIX''\)')
      RETURN 1
      end subroutine readSparseMatrix
      end module modSparseMatrix
