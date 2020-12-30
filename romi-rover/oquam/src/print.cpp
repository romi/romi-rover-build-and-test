/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include "print.h"

namespace romi {

        void indent_text(membuf_t *text, int size)
        {
                for (int i = 0; i < size; i++)
                        membuf_put(text, ' ');
        }
        
        void print(Section& section, membuf_t *text, int indent)
        {
                indent_text(text, indent);
                membuf_printf(text, "{\n");
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"start-time\": %0.6f,\n",
                              section.start_time);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"duration\": %0.6f,\n",
                              section.duration);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"p0\": [%0.4f, %0.4f, %0.4f],\n",
                              section.p0[0], section.p0[1], section.p0[2]);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"p1\": [%0.4f, %0.4f, %0.4f],\n",
                              section.p1[0], section.p1[1], section.p1[2]);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"v0\": [%0.3f, %0.3f, %0.3f],\n",
                              section.v0[0], section.v0[1], section.v0[2]);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"v1\": [%0.3f, %0.3f, %0.3f],\n",
                              section.v1[0], section.v1[1], section.v1[2]);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"a\": [%0.3f, %0.3f, %0.3f]\n",
                              section.a[0], section.a[1], section.a[2]);
                
                indent_text(text, indent);
                membuf_printf(text, "}");
        }
        
        void print(Section& section)
        {
                membuf_t *text = new_membuf();
                print(section, text);
                membuf_append_zero(text);
                printf("%s\n", membuf_data(text));
                delete_membuf(text);
        }

        void print(ATDC *atdc, membuf_t *text, int indent)
        {
                indent_text(text, indent);
                membuf_printf(text, "{\n");
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"accelerate\":\n");
                print(atdc->accelerate, text, indent+2);
                membuf_printf(text, ",\n");
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"travel\":\n");
                print(atdc->travel, text, indent+2);
                membuf_printf(text, ",\n");
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"decelerate\":\n");
                print(atdc->decelerate, text, indent+2);
                membuf_printf(text, ",\n");
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"curve\":\n");
                print(atdc->curve, text, indent+2);
                membuf_printf(text, "\n");
                
                indent_text(text, indent);
                membuf_printf(text, "}");
        }

        void print(ATDC *atdc)
        {
                membuf_t *buf = new_membuf();
                print(atdc, buf);
                membuf_append_zero(buf);
                printf("%s\n", membuf_data(buf));
                delete_membuf(buf);
        }

        void print(Move& move, membuf_t *text)
        {
                membuf_printf(text,
                              "{\"x\": %.5f, \"y\": %.5f, \"z\": %.5f, \"v\": %.5f}",
                              move.p[0], move.p[1], move.p[2], move.v);
        }
        
        void print_moves(Script& script, membuf_t *text)
        {
                membuf_printf(text, "  \"moves\": [\n");
                for (size_t i = 0; i < script.count_moves(); i++) {
                        indent_text(text, 4);
                        print(script.get_move(i), text);
                        if (i == script.count_moves() - 1) 
                                membuf_printf(text, "\n");
                        else 
                                membuf_printf(text, ",\n");
                }
                membuf_printf(text, "  ],\n");
        }

        void print(Segment& segment, membuf_t *text, int indent)
        {
                indent_text(text, indent);
                membuf_printf(text, "{\n");
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"p0\": [%0.4f, %0.4f, %0.4f],\n",
                              segment.p0[0], segment.p0[1], segment.p0[2]);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"p1\": [%0.4f, %0.4f, %0.4f],\n",
                              segment.p1[0], segment.p1[1], segment.p1[2]);
                
                indent_text(text, indent+2);
                membuf_printf(text, "\"v\": [%0.3f, %0.3f, %0.3f]\n",
                              segment.v[0], segment.v[1], segment.v[2]);
                
                indent_text(text, indent);
                membuf_printf(text, "}");
        }

        void print_segments(Script& script, membuf_t *text)
        {
                Segment *s = script.segments;
                membuf_printf(text, "  \"segments\": [\n");
                while (s != NULL) {
                        print(*s, text, 4);
                        s = s->next;
                        if (s) 
                                membuf_printf(text, ",\n");
                        else 
                                membuf_printf(text, "\n");
                }
                membuf_printf(text, "  ],\n");
        }
        
        void print_atdc(Script& script, membuf_t *text)
        {
                ATDC *s = script.atdc;
                membuf_printf(text, "  \"atdc\": [\n");
                while (s != NULL) {
                        print(s, text, 4);
                        s = s->next;
                        if (s) 
                                membuf_printf(text, ",\n");
                        else 
                                membuf_printf(text, "\n");
                }
                membuf_printf(text, "  ]");
        }
        
        void print_slices(Script& script, membuf_t *text)
        {
                membuf_printf(text, "  \"slices\": [\n");
                for (size_t k = 0; k < script.slices.size(); k++) {
                        print(script.slices[k], text, 4);
                        if (k == script.slices.size() - 1) 
                                membuf_printf(text, "\n");
                        else 
                                membuf_printf(text, ",\n");
                }
                membuf_printf(text, "  ]\n");
        }

        void print(Script& script, membuf_t *text, bool include_slices)
        {
                membuf_printf(text, "{\n");
                
                print_moves(script, text);
                
                print_segments(script, text);
                
                print_atdc(script, text);
                
                if (include_slices) {
                        membuf_printf(text, ",\n");
                        print_slices(script, text);
                } else {
                        membuf_printf(text, "\n");
                }
                membuf_printf(text, "}\n");
        }

        void print(Script& script, bool include_slices)
        {
                membuf_t *buf = new_membuf();
                print(script, buf, include_slices);
                membuf_append_zero(buf);
                printf("%s\n", membuf_data(buf));
                delete_membuf(buf);
        }
}



