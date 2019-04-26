// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package publish

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"testing"

	"fuchsia.googlesource.com/pm/build"
	"fuchsia.googlesource.com/pm/repo"
)

var cfg = build.TestConfig()

func TestPublishArchive(t *testing.T) {
	depfilePath := filepath.Join(cfg.OutputDir, "depfile.d")

	archivePath, err := makePackageArchive()
	if err != nil {
		t.Fatal(err)
	}
	defer os.Remove(archivePath)

	repoDir, err := ioutil.TempDir("", "pm-publish-test-repo")
	if err != nil {
		t.Fatal(err)
	}
	defer os.RemoveAll(repoDir)

	if err := Run(cfg, []string{"-repo", repoDir, "-depfile", depfilePath, "-a", "-f", archivePath}); err != nil {
		t.Fatal(err)
	}

	for _, jsonPath := range []string{"timestamp.json", "targets.json", "snapshot.json"} {
		f, err := os.Open(filepath.Join(repoDir, "repository", jsonPath))
		if err != nil {
			t.Fatal(err)
		}
		defer f.Close()
		m := make(map[string]json.RawMessage)
		if err := json.NewDecoder(f).Decode(&m); err != nil {
			t.Fatal(err)
		}
	}

	assertHasTestPackage(t, repoDir)

	outName, inputPaths := readDepfile(depfilePath)
	if ex := filepath.Join(repoDir, "repository", "timestamp.json"); ex != outName {
		t.Errorf("depfile output: got %q, want %q", outName, ex)
	}

	if len(inputPaths) != 1 {
		t.Fatalf("got %#v, wanted one input", inputPaths)
	}

	if inputPaths[0] != archivePath {
		t.Errorf("depfile inputs: %#v != %#v", inputPaths[0], archivePath)
	}
}

func readDepfile(depfilePath string) (string, []string) {
	b, err := ioutil.ReadFile(depfilePath)
	if err != nil {
		panic(err)
	}
	parts := bytes.SplitN(b, []byte(": "), 2)
	outName := string(parts[0])
	inputs := bytes.Split(bytes.TrimSpace(parts[1]), []byte(" "))
	inputPaths := []string{}
	for _, input := range inputs {
		inputPaths = append(inputPaths, string(input))
	}
	for i, input := range inputPaths {
		var err error
		inputPaths[i], err = strconv.Unquote(input)
		if err != nil {
			panic(err)
		}
	}
	return outName, inputPaths
}

func assertHasTestPackage(t *testing.T, repoDir string) {
	repo, err := repo.New(repoDir)
	if err != nil {
		panic(err)
	}
	dataFiles, err := repo.Targets()
	if err != nil {
		panic(err)
	}
	_, ok := dataFiles["/testpackage/0"]
	if !ok {
		t.Fatalf("package not found: %q in %#v", "testpackage", dataFiles)
	}
}

func makePackageArchive() (string, error) {
	build.BuildTestPackage(cfg)
	f, err := ioutil.TempFile("", "testpackage-0")
	if err != nil {
		return "", err
	}
	f.Close()
	return f.Name() + ".far", build.Archive(cfg, f.Name())
}
