#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#****************************************************************************
#   mfGraph Library for manipulating and rendering DOT graphs               #
#   Copyright (c) 2005 Michael Foetsch                                      #
#                                                                           #
#  This library is free software; you can redistribute it and/or            #
#  modify it under the terms of the GNU Lesser General Public               #
#  License as published by the Free Software Foundation; either             #
#  version 2.1 of the License, or (at your option) any later version.       #
#                                                                           #
#  This library is distributed in the hope that it will be useful,          #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of           #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU         #
#  Lesser General Public License for more details.                          #
#                                                                           #
#  You should have received a copy of the GNU Lesser General Public         #
#  License along with this library; if not, write to the Free Software      #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  #
#                                                                           #
#  Contact:    foetsch@yahoo.com                                            #
#              http://mfgraph.sourceforge.net/                              #
#****************************************************************************

from mfgraph import pymfg
from mfgraph.pymfg import deref

class ObjectSelection(object):
    """@brief Helper class for MfgraphWindow to handle object selection.

    Properties:
    @li @c SelectableObjectTypes - object types that are selectable;
        default = [pymfg.OBJECT_NODE, pymfg.OBJECT_EDGE, pymfg.OBJECT_SUBGRAPH];
        already selected objects are not affected when you set this
    @li @c SelectedObjects - read-only; a list of selected objects;
        a copy is returned; the last selected object is the first in the list
    """
    def __init__(self,
                 draw_graph=None,
                 selection_changed_callback=None):
        """@brief Init from pymfg.DrawGraph"""
        if draw_graph is not None:
            self.__BuildObjectList(draw_graph)

        self.__m_selected_objects = []
        self.__m_selection_changed = selection_changed_callback

        # properties
        self.__SetSelectableObjectTypes([pymfg.OBJECT_NODE,
                                         pymfg.OBJECT_EDGE,
                                         pymfg.OBJECT_SUBGRAPH])

    def GetFirstSelectedObject(self):
        """@brief Return the first selected pymfg.Object, or None."""
        try:
            return self.__m_selected_objects[0]
        except IndexError:
            return None

    def GetFollowingObject(self, obj):
        """@brief Get following object (selected or unselected) by going through
            bounding boxes from left to right and from top to bottom.

        Used by MfgraphWindow to allow TAB-cycling through objects.

        @param obj - pymfg.Object
        @return Following pymfg.Object or first pymfg.Object if @a obj is None

        @remarks Only those objects are considered whose types appear in
        @c SelectableObjectTypes. If @a obj is already the last selectable object,
        the first selectable object from the list is returned. If there are no
        selectable objects, None is returned.
        """
        obj = _CastToObject(obj)
        if obj is None:
            start_index = 0
        else:
            start_index = self.__IndexInObjectList(obj) + 1
        for i in xrange(start_index, start_index + len(self.__m_object_list)):
            following_idx = i % len(self.__m_object_list)
            following_obj = self.__m_object_list[following_idx]
            if following_obj.Type() in self.__m_selectable_object_types:
                return following_obj
        else:
            return None

    def GetPreviousObject(self, obj):
        """@brief Opposite of GetFollowingObject."""
        obj = _CastToObject(obj)
        if obj is None:
            start_index = 0
        else:
            start_index = len(self.__m_object_list) - 1 \
                          - self.__IndexInObjectList(obj) + 1
        for i in xrange(start_index, start_index + len(self.__m_object_list)):
            following_idx = i % len(self.__m_object_list)
            following_obj = self.__m_object_list[len(self.__m_object_list) - 1
                                                 - following_idx]
            if following_obj.Type() in self.__m_selectable_object_types:
                return following_obj
        else:
            return None

    def SelectSingleObject(self, obj):
        """@brief Set SelectedObjects to contain only the specified object.

        @param obj - pymfg.Object to select; if @a obj is None or the
            object type is not in SelectableObjectTypes, the effect is the same
            as if SelectAllObjects(False) had been called
        """
        obj = _CastToObject(obj)
        if obj is None or obj.Type() not in self.__m_selectable_object_types:
            self.__m_selected_objects = []
        else:
            self.__m_selected_objects = [obj]
        if callable(self.__m_selection_changed):
            self.__m_selection_changed()

    def SetObjectSelected(self, obj, selected=True):
        """@brief Set selection state (True or False) of given object.

        @param obj - pymfg.Object
        @param selected - if True and the object is not already selected and
            the object type is in SelectableObjectTypes, then @a obj
            is appended to SelectedObjects; if False and object is selected,
            @a obj is removed from SelectedObjects
        """
        obj = _CastToObject(obj)
        if selected:
            if not self.IsObjectSelected(obj) \
               and obj.Type() in self.__m_selectable_object_types:
                self.__m_selected_objects.insert(0, obj)
        else:
            self.__m_selected_objects = filter(lambda o: o.this != obj.this,
                                               self.__m_selected_objects)
        if callable(self.__m_selection_changed):
            self.__m_selection_changed()

    def IsObjectSelectable(self, obj):
        """@brief Return True if the object's type is in SelectableObjectTypes.

        @param obj - pymfg.Object
        """
        return _CastToObject(obj).Type() in self.__m_selectable_object_types

    def IsObjectSelected(self, obj):
        """@brief Return selection state of the specified object."""
        return _CastToObject(obj).this in map(lambda o: o.this,
                                              self.__m_selected_objects)

    def SelectAllObjects(self, selected=True):
        """@brief Select all objects; same as SetObjectSelected for all objects.

        @param selected - if True, all objects are selected; if False, all
            objects are unselected

        @remarks
        Only those objects are selected whose object type is in SelectableObjectTypes.
        """
        if selected:
            self.__m_selected_objects \
                = filter(lambda o: o.Type() in self.__m_selectable_object_types,
                         self.__m_object_list)
        else:
            self.__m_selected_objects = []
        if callable(self.__m_selection_changed):
            self.__m_selection_changed()

    def __IndexInObjectList(self, obj):
        obj = _CastToObject(obj)
        for i, o in enumerate(self.__m_object_list):
            if o.this == obj.this:
                return i
        else:
            raise ValueError(obj)

    def __BuildObjectList(self, draw_graph):
        """@brief Fill list self.__m_object_list with pymfg.Object objects,
            sorted by geometric position according to bounding rect.

        The sorting determines how GetFollowingObject works.
        """
        object_rectangle_list = []
        for obj in list(draw_graph.GetNodeList()) \
                   + list(draw_graph.GetEdgeList()) \
                   + list(draw_graph.GetSubgraphList()):
            draw_object = obj.AsDrawObject()
            object_rectangle_list.append((_CastToObject(obj),
                                          draw_object.GetBoundingRect()))
        object_rectangle_list.sort(_CompareRects)
        self.__m_object_list = map(lambda (obj, rect): obj,
                                   object_rectangle_list)


    # Properties.

    def __GetSelectableObjectTypes(self):
        return self.__m_selectable_object_types

    def __SetSelectableObjectTypes(self, new):
        self.__m_selectable_object_types = new[:]

    def __GetSelectedObjects(self):
        return self.__m_selected_objects[:]

    SelectableObjectTypes = property(fget=__GetSelectableObjectTypes,
                                     fset=__SetSelectableObjectTypes)

    SelectedObjects = property(fget=__GetSelectedObjects)


def _CompareRects((obj1, rect1), (obj2, rect2)):
    r1 = (_RoundTo(5, -rect1.top), rect1.left)
    r2 = (_RoundTo(5, -rect2.top), rect2.left)
    if r1 < r2:
        return -1
    elif r1 == r2:
        return 0
    else:
        return 1

def _RoundTo(m, i):
    # round i to multiple of m
    return int(round(i / float(m)) * m)

def _CastToObject(obj):
    # cast a Node, Edge, or Subgraph to an Object (must go through
    # DrawInfo until pymfg supports it directly)
    if obj is None:
        return None
    else:
        return obj.AsDrawObject().GetDrawInfo(
            pymfg.DrawInfo.ENTITY_OBJECT).GetOwnerAsObject()

