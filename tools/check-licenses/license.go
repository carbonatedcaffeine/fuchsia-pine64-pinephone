// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package checklicenses

import (
	"regexp"
	"strings"
	"sync"
)

// License contains a searchable regex pattern for finding file matches in
// tree.
//
// The category field is the .lic name
type License struct {
	pattern   *regexp.Regexp
	category  string
	validType bool

	mu           sync.Mutex
	matches      map[string]*Match
	matchChannel chan *Match
}

// Match is used to store a single match result alongside the License along
// with a list of all matching files
type Match struct {
	authors string
	value   string
	files   []string
}

// LicenseFindMatch runs concurrently for all licenses, synchronizing result production for subsequent consumption
func (license *License) LicenseFindMatch(index int, data []byte, sm *sync.Map, wg *sync.WaitGroup) {
	defer wg.Done()
	sm.Store(index, license.pattern.Find(data))
}

func (l *License) appendFile(path string) {
	p := l.pattern.String()
	a := parseAuthor(p)
	// Replace < and > so that it doesn't cause special character highlights.
	a = strings.ReplaceAll(a, "<", "&lt")
	a = strings.ReplaceAll(a, ">", "&gt")
	l.AddMatch(&Match{authors: a, value: p, files: []string{path}})
}

func (l *License) AddMatch(m *Match) {
	l.matchChannel <- m
}

func (l *License) MatchChannelWorker() {
	for m := range l.matchChannel {
		if m == nil {
			break
		}
		l.mu.Lock()
		if _, ok := l.matches[m.authors]; !ok {
			// Replace < and > so that it doesn't cause special character highlights.
			p := strings.ReplaceAll(m.value, "<", "&lt")
			p = strings.ReplaceAll(p, ">", "&gt")
			l.matches[m.authors] = &Match{authors: m.authors, value: p, files: m.files}
		} else {
			l.matches[m.authors].files = append(l.matches[m.authors].files, m.files...)
		}
		l.mu.Unlock()
	}
}

var reAuthor = regexp.MustCompile(`(?i)Copyright(?: ©| \((?:C)\))? [\d]{4}(?:\s|,|-|[\d]{4})*(.*)?`)

func parseAuthor(l string) string {
	m := reAuthor.FindStringSubmatch(l)
	if m == nil {
		return ""
	}
	a := strings.TrimSpace(m[1])
	const rights = " All rights reserved"
	if strings.HasSuffix(a, rights) {
		a = a[:len(a)-len(rights)]
	}
	return a
}
