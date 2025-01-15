/* Point.h
 * Copyright (C) 2010, Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Point Description in a Binary Relation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Contacting the author:
 *   Miguel A Martinez-Prieto:  migumar2@infor.uva.es
 */


#ifndef _POINT_H
#define _POINT_H

namespace cds_static
{
	/** Point
	 *   Describes a pair (row,column) for a Binary Relation
	 *
	 *   @author Miguel A. Martinez-Prieto
	 */
	class Point
	{	
		public:
			int left;	// Left (row) component
			int right;	// Right (column) component

			Point(){};
			Point(int left, int right)
			{
				this->left = left; 
				this->right = right;
			}
			~Point(){};

			bool static sortPointsByRow(const Point& c1, const Point& c2)
			{
				if ((c1.left) < (c2.left)) return true;
				else
				{
					if ((c1.left) > (c2.left)) return false;
					else
					{
						if ((c1.right) < (c2.right)) return true;
						else return false;
					}
				}

				return true;
			}

			bool static sortPointsByColumn(const Point& c1, const Point& c2)
			{
				if ((c1.right) < (c2.right)) return true;
				else
				{
					if ((c1.right) > (c2.right)) return false;
					else
					{
						if ((c1.left) < (c2.left)) return true;
						else return false;
					}
				}

				return true;
			}

	};
};

#include "Rule.h"

#endif  /* _POINT_H */
