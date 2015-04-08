/* Documenttitle and purpose:                                       */
/*   Implementation of the UN*X wildcards in C. So they are         */
/*   available in a portable way and can be used whereever          */
/*   needed.                                                        */
/*                                                                  */
/* Version:                                                         */
/*   1.1                                                            */
/*                                                                  */
/* Test cases:                                                      */
/*   Please look into the Shellscript testwildcards.main            */
/*                                                                  */
/* Author(s):                                                       */
/*   Florian Schintke (schintke@cs.tu-berlin.de)                    */
/*                                                                  */
/* Tester:                                                          */
/*   Florian Schintke (schintke@cs.tu-berlin.de)                    */
/*                                                                  */
/* Testing state:                                                   */
/*   tested with the atac tool                                      */
/*   Latest test state is documented in testwildcards.report        */
/*   The program that tested this functions is available as         */
/*   testwildcards.c                                                */
/*                                                                  */
/* Dates:                                                           */
/*   First editing: unknown, but before 04/02/1997                  */
/*   Last Change  : 11/10/1998                                      */
/*                                                                  */
/* Known Bugs:                                                      */
/*                                                                  */

/* Changes Made for ICIS by Rolando M Casumpang                     */
/*    replaced all * by %                                           */
/*    replaced all ? by _                                           */
/* Encode & Decode Routines made by Arllet M. Portugal              */ 
#include <stdio.h>
#include "wildcards.h"

int set (char **wildcard, char **test);
/* Scans a set of characters and returns 0 if the set mismatches at this */
/* position in the teststring and 1 if it is matching                    */
/* wildcard is set to the closing ] and test is unmodified if mismatched */
/* and otherwise the char pointer is pointing to the next character      */

int asterisk (char **wildcard, char **test);
/* scans an asterisk */

int wildcardfit (char *wildcard, char *test)
{
  int fit = 1;
  
  for (; ('\000' != *wildcard) && (1 == fit) && ('\000' != *test); wildcard++)
    {
      switch (*wildcard)
        {
        case '[':
	  wildcard++; /* leave out the opening square bracket */ 
          fit = set (&wildcard, &test);
	  /* we don't need to decrement the wildcard as in case */
	  /* of asterisk because the closing ] is still there */
          break;
        //case '?':
        case '_':
          test++;
          break;
        //case '*':
        case '%':
          fit = asterisk (&wildcard, &test);
	  /* the asterisk was skipped by asterisk() but the loop will */
	  /* increment by itself. So we have to decrement */
	  wildcard--;
          break;
        default:
          fit = (int) (*wildcard == *test);
          test++;
        }
    }
  //while ((*wildcard == '*') && (1 == fit)) 
  while ((*wildcard == '%') && (1 == fit)) 
    /* here the teststring is empty otherwise you cannot */
    /* leave the previous loop */ 
    wildcard++;
  return (int) ((1 == fit) && ('\0' == *test) && ('\0' == *wildcard));
}

int set (char **wildcard, char **test)
{
  int fit = 0;
  int negation = 0;
  int at_beginning = 1;

  if ('!' == **wildcard)
    {
      negation = 1;
      (*wildcard)++;
    }
  while ((']' != **wildcard) || (1 == at_beginning))
    {
      if (0 == fit)
        {
          if (('-' == **wildcard) 
              && ((*(*wildcard - 1)) < (*(*wildcard + 1)))
              && (']' != *(*wildcard + 1))
	      && (0 == at_beginning))
            {
              if (((**test) >= (*(*wildcard - 1)))
                  && ((**test) <= (*(*wildcard + 1))))
                {
                  fit = 1;
                  (*wildcard)++;
                }
            }
          else if ((**wildcard) == (**test))
            {
              fit = 1;
            }
        }
      (*wildcard)++;
      at_beginning = 0;
    }
  if (1 == negation)
    /* change from zero to one and vice versa */
    fit = 1 - fit;
  if (1 == fit) 
    (*test)++;

  return (fit);
}

int asterisk (char **wildcard, char **test)
{
  /* Warning: uses multiple returns */

  int fit = 1;
  char *oldwildcard, *oldtest;

  /* erase the leading asterisk */
  (*wildcard)++; 
  while (('\000' != (**test))
	 //&& (('?' == **wildcard) 
	 //    || ('*' == **wildcard)))
	 && (('_' == **wildcard) 
	     || ('%' == **wildcard)))
    {
      //if ('?' == **wildcard) 
     if ('_' == **wildcard)
	(*test)++;
      (*wildcard)++;
    }
  /* Now it could be that test is empty and wildcard contains */
  /* aterisks. Then we delete them to get a proper state */
  //while ('*' == (**wildcard))
  while ('%' == (**wildcard))
    (*wildcard)++;

  if (('\0' == (**test)) && ('\0' != (**wildcard)))
    return (fit = 0);
  if (('\0' == (**test)) && ('\0' == (**wildcard)))
    return (fit = 1); 
  else
    {
      /* Neither test nor wildcard are empty!          */
      /* the first character of wildcard isn't in [*?] */
      oldwildcard = *wildcard;
      oldtest = *test;
      do 
	{
	  if (0 == wildcardfit(*wildcard, (*test)))
	    oldtest++;
	  *wildcard = oldwildcard;
	  *test = oldtest;
	  /* skip as much characters as possible in the teststring */
	  /* stop if a character match occurs */
	  while (((**wildcard) != (**test)) 
		 && ('['  != (**wildcard))
		 && ('\0' != (**test)))
	    (*test)++;
	  oldwildcard = *wildcard;
	  oldtest = *test;
	}
      while ((('\0' != **test))? 
	     (0 == wildcardfit (*wildcard, (*test))) 
	     : (0 != (fit = 0)));
      if (('\0' == **test) && ('\0' == **wildcard))
	fit = 1;
      return (fit);
    }
}


